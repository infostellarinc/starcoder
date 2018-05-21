/*
 * Starcoder - a server to read/write data from/to the stars, written in Go.
 * Copyright (C) 2018 InfoStellar, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package server

import (
	"errors"
	"fmt"
	pb "github.com/infostellarinc/starcoder/api"
	"github.com/infostellarinc/starcoder/cqueue"
	"github.com/sbinet/go-python"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"go.uber.org/zap"
)

const defaultQueueSize = 1048576

type Starcoder struct {
	flowgraphDir              string
	gilState                  python.PyGILState
	streamHandlers            map[*streamHandler]bool // registered stream handlers
	registerStreamHandler     chan *streamHandler
	deregisterStreamHandler   chan *streamHandler
	closeAllStreamsChannel    chan chan bool
	tempModule                string
	filepathToModAndClassName map[string]*moduleAndClassNames
	compileLock               sync.Mutex
	log                       *zap.SugaredLogger
}

type moduleAndClassNames struct {
	moduleName string
	className  string
}

func NewStarcoderServer(flowgraphDir string, log *zap.SugaredLogger) *Starcoder {
	err := python.Initialize()
	if err != nil {
		log.Fatalf("failed to initialize python: %v", err)
	}

	// This call releases the GIL so it can be acquired from different threads i.e. everywhere else
	// See these links:
	// https://stackoverflow.com/questions/15470367/
	// https://stackoverflow.com/questions/10625584/
	python.PyEval_SaveThread()

	s := &Starcoder{
		flowgraphDir:              flowgraphDir,
		streamHandlers:            make(map[*streamHandler]bool),
		registerStreamHandler:     make(chan *streamHandler),
		deregisterStreamHandler:   make(chan *streamHandler),
		closeAllStreamsChannel:    make(chan chan bool),
		filepathToModAndClassName: make(map[string]*moduleAndClassNames),
		log: log,
	}

	tempDir, err := ioutil.TempDir("", "starcoder")
	if err != nil {
		log.Fatalf("failed to create temporary directory %v", err)
	}
	s.tempModule = tempDir

	s.withPythonInterpreter(func() {
		// Append module directory to sys.path
		log.Infof("Appending %v to sys.path", tempDir)
		sysPath := python.PySys_GetObject("path")
		if sysPath == nil {
			err = errors.New(getExceptionString())
			log.Fatalf("failed to get sys.path %v", err)
		}
		moduleDir := python.PyString_FromString(tempDir)
		if moduleDir == nil {
			err = errors.New(getExceptionString())
			log.Fatalf("Python error: %v", err)
		}
		defer moduleDir.DecRef()
		err = python.PyList_Append(sysPath, moduleDir)
		if err != nil {
			log.Fatalf("Python error: %v", err)
		}
	})

	go func(s *Starcoder) {
		for {
			select {
			case sh := <-s.registerStreamHandler:
				s.streamHandlers[sh] = true
			case sh := <-s.deregisterStreamHandler:
				if _, ok := s.streamHandlers[sh]; ok {
					sh.Close()
					delete(s.streamHandlers, sh)
				}
			case respCh := <-s.closeAllStreamsChannel:
				for sh := range s.streamHandlers {
					sh.Close()
				}
				respCh <- true
				return
			}
		}
	}(s)

	return s
}

func (s *Starcoder) closeAllStreams() {
	respCh := make(chan bool)
	s.closeAllStreamsChannel <- respCh
	<-respCh
}

type streamHandler struct {
	starcoder         *Starcoder
	stream            pb.Starcoder_RunFlowgraphServer
	flowGraphInstance *python.PyObject
	observableCQueues map[string]*cqueue.CStringQueue
	clientFinished    bool
	streamError       error
	mustCloseMutex    sync.Mutex
	wg                sync.WaitGroup
	closeChannel      chan chan bool
	log *zap.SugaredLogger
}

func newStreamHandler(sc *Starcoder, stream pb.Starcoder_RunFlowgraphServer, fgInstance *python.PyObject, observableCQueues map[string]*cqueue.CStringQueue, log *zap.SugaredLogger) *streamHandler {
	sh := &streamHandler{
		starcoder:         sc,
		stream:            stream,
		flowGraphInstance: fgInstance,
		observableCQueues: observableCQueues,
		clientFinished:    false,
		streamError:       nil,
		mustCloseMutex:    sync.Mutex{},
		wg:                sync.WaitGroup{},
		closeChannel:      make(chan chan bool),
		log: log,
	}

	go func() {
		for {
			// Beyond the first packet, we don't care what the client
			// is sending us until it wants to end the connection.
			// TODO: Eventually we would want to let the client send
			// info to the flow graph during runtime. This will be the
			// ideal place to receive/process it.
			_, err := stream.Recv()
			if err == io.EOF {
				// Client is done listening
				break
			}
			if err != nil {
				log.Errorf("Received error from client: %v", err)
				sh.finish(err)
				return
			}
		}
		sh.finish(nil)
	}()

	for k, cQueue := range sh.observableCQueues {
		blockName := k
		q := cQueue
		go func() {
			sh.wg.Add(1)
			defer sh.wg.Done()
			for {
				// TODO: Convert PMT to a gRPC native data structure.
				// Use built-in PMT serialization for now.
				bytes := []byte(q.BlockingPop())
				// Could be woken up spuriously or by something else calling q.Close()
				if len(bytes) != 0 {
					if err := stream.Send(&pb.RunFlowgraphResponse{
						BlockId: blockName,
						Payload: bytes,
					}); err != nil {
						log.Errorf("Error sending stream: %v", err)
						sh.finish(err)
						return
					}
				}
				if sh.finished() {
					// Send the rest of the bytes if any are left
					for bytes = []byte(q.Pop()); len(bytes) != 0; bytes = []byte(q.Pop()) {
						log.Info(bytes)
						if err := stream.Send(&pb.RunFlowgraphResponse{
							BlockId: blockName,
							Payload: bytes,
						}); err != nil {
							log.Errorf("Error sending stream: %v", err)
						}
					}
					return
				}
			}
		}()
	}

	sh.wg.Add(1)
	return sh
}

func (sh *streamHandler) Wait() {
	sh.wg.Wait()
}

// Called within streamHandler whenever an error happens or client finished.
// Signals starcoder server that it needs to end this stream handler
func (sh *streamHandler) finish(err error) {
	sh.mustCloseMutex.Lock()
	defer sh.mustCloseMutex.Unlock()
	if err == nil {
		sh.clientFinished = true
	}
	sh.streamError = err
	go func() {
		sh.starcoder.deregisterStreamHandler <- sh
	}()
}

// Called by goroutines of streamHandler when they want to know when to close
func (sh *streamHandler) finished() bool {
	sh.mustCloseMutex.Lock()
	defer sh.mustCloseMutex.Unlock()
	return sh.clientFinished || sh.streamError != nil
}

// Must only be called by starcoder server
func (sh *streamHandler) Close() {
	defer func() {
		for _, cQueue := range sh.observableCQueues {
			cQueue.Close()
		}
		sh.wg.Done()
	}()
	// TODO: Make this call unblock by getting rid of `wait`
	err := sh.starcoder.stopFlowGraph(sh.flowGraphInstance)
	if err != nil {
		sh.log.Errorf("Error stopping flowgraph: %v", err)
		sh.finish(err)
	}
}

func (s *Starcoder) RunFlowgraph(stream pb.Starcoder_RunFlowgraphServer) error {
	in, err := stream.Recv()
	if err == io.EOF {
		return nil
	}
	if err != nil {
		s.log.Errorf("Error receiving gRPC: %v", err)
		return err
	}

	var startRequest *pb.StartFlowgraphRequest

	if x, ok := in.Request.(*pb.RunFlowgraphRequest_StartFlowgraphRequest); !ok {
		return errors.New("first request in stream is not StartFlowgraphRequest")
	} else {
		startRequest = x.StartFlowgraphRequest
	}

	inFileAbsPath := filepath.Join(s.flowgraphDir, startRequest.GetFilename())

	modAndClass, err := s.compileGrc(inFileAbsPath)
	if err != nil {
		s.log.Errorf("Error compiling flowgraph: %v", err)
		return err
	}

	flowGraphInstance, observableCQueues, err := s.startFlowGraph(modAndClass, startRequest)
	if err != nil {
		s.log.Errorf("Error starting flowgraph: %v", err)
		return err
	}

	defer func() {
		s.withPythonInterpreter(func() {
			flowGraphInstance.DecRef()
		})
		for _, q := range observableCQueues {
			q.Delete()
		}
	}()

	sh := newStreamHandler(s, stream, flowGraphInstance, observableCQueues, s.log)
	s.registerStreamHandler <- sh
	sh.Wait()

	if sh.streamError != nil {
		s.log.Errorf("Finished runFlowgraph with error: %v", sh.streamError)
	}
	return sh.streamError
}

func (s *Starcoder) compileGrc(path string) (*moduleAndClassNames, error) {
	// This lock is needed to avoid compiling the same .grc file more than once
	// and to prevent race conditions when copying things over to tempModule.
	s.compileLock.Lock()
	defer s.compileLock.Unlock()
	if val, ok := s.filepathToModAndClassName[path]; ok {
		s.log.Infof(
			"Already compiled %v: Using module name %v and class name %v",
			path, val.moduleName, val.className)
		return val, nil
	}

	if _, err := os.Stat(path); os.IsNotExist(err) {
		return nil, err
	}

	var moduleName, className string

	if strings.HasSuffix(path, ".grc") {
		grccPath, err := exec.LookPath("grcc")
		if err != nil {
			return nil, err
		}

		// Create temporary directory to store compiled .py file.
		// We need this because we can't control the output filename
		// of the compiled file.
		tempDir, err := ioutil.TempDir("", "starcoder")
		if err != nil {
			return nil, err
		}
		defer os.RemoveAll(tempDir)

		grccCmd := exec.Command(grccPath, "-d", tempDir, path)
		err = grccCmd.Run()
		if err != nil {
			return nil, err
		}

		files, err := ioutil.ReadDir(tempDir)
		if err != nil {
			return nil, err
		}

		if len(files) != 1 {
			return nil, errors.New(fmt.Sprintf(
				"Unexpected number of files output by GRCC: %v", len(files)))
		}

		s.log.Infof("Successfully compiled %v to %v", path, files[0].Name())

		//Rename the moduleName to something unique so it won't overwrite other files
		f, err := ioutil.TempFile(tempDir, strings.TrimSuffix(files[0].Name(), ".py"))
		f.Close()
		uniqueModuleName := filepath.Base(f.Name())

		err = os.Rename(
			filepath.Join(tempDir, files[0].Name()),
			filepath.Join(s.tempModule, uniqueModuleName+".py"),
		)
		if err != nil {
			return nil, err
		}
		s.log.Infof("Renamed %v to %v", files[0].Name(), uniqueModuleName+".py")

		moduleName = uniqueModuleName
		className = strings.TrimSuffix(files[0].Name(), ".py")
	} else if strings.HasSuffix(path, ".py") {
		// TODO: Support directly using already compiled .py files
		return nil, errors.New("unsupported file type")
	} else {
		return nil, errors.New("unsupported file type")
	}
	retVal := &moduleAndClassNames{
		moduleName: moduleName,
		className:  className,
	}
	s.filepathToModAndClassName[path] = retVal
	return retVal, nil
}

func (s *Starcoder) startFlowGraph(modAndImport *moduleAndClassNames, request *pb.StartFlowgraphRequest) (*python.PyObject, map[string]*cqueue.CStringQueue, error) {
	var flowGraphInstance *python.PyObject
	var observableCQueue map[string]*cqueue.CStringQueue
	var err error

	s.withPythonInterpreter(func() {
		// Import module
		module := python.PyImport_ImportModule(modAndImport.moduleName)
		if module == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer module.DecRef()

		// Find top_block subclass
		// GRC compiled python scripts have the top block class name equal to the python filename.
		flowgraphClass := module.GetAttrString(modAndImport.className)
		if flowgraphClass == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer flowgraphClass.DecRef()
		gnuRadioModule := python.PyImport_ImportModule("gnuradio")
		if gnuRadioModule == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer gnuRadioModule.DecRef()
		grModule := gnuRadioModule.GetAttrString("gr")
		if grModule == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer grModule.DecRef()
		topBlock := grModule.GetAttrString("top_block")
		if topBlock == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer topBlock.DecRef()

		// Verify top_block subclass
		isSubclass := flowgraphClass.IsSubclass(topBlock)
		if isSubclass == 0 {
			err = errors.New(fmt.Sprintf(
				"Top block class %v is not a "+
					"subclass of gnuradio.gr.top_block", modAndImport.className))
			return

		} else if isSubclass == -1 {
			err = errors.New(getExceptionString())
			return
		}

		kwArgs := python.PyDict_New()
		if kwArgs == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer kwArgs.DecRef()

		fillDictWithParameters(kwArgs, request.GetParameters())

		emptyTuple := python.PyTuple_New(0)
		if emptyTuple == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer emptyTuple.DecRef()

		flowGraphInstance = flowgraphClass.Call(emptyTuple, kwArgs)
		if flowGraphInstance == nil {
			err = errors.New(getExceptionString())
			return
		}

		callReturn := flowGraphInstance.CallMethod("start")
		if callReturn == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer callReturn.DecRef()

		observableCQueue = make(map[string]*cqueue.CStringQueue)

		// Look for any Enqueue Message Sink blocks
		starcoderModule := python.PyImport_ImportModule("starcoder")
		if starcoderModule == nil {
			s.log.Warn("gr-starcoder module not found. There are no blocks to observe")
		} else {
			defer starcoderModule.DecRef()
			flowGraphDict := flowGraphInstance.GetAttrString("__dict__")
			if flowGraphDict == nil {
				err = errors.New(getExceptionString())
				return
			}
			defer flowGraphDict.DecRef()

			iter := 0
			key := python.PyString_FromString("")
			defer key.DecRef()
			val := python.PyString_FromString("")
			defer val.DecRef()
			for python.PyDict_Next(flowGraphDict, &iter, &key, &val) {
				// Verify instance has register_starcoder_queue
				hasStarcoderObserve := val.HasAttrString("register_starcoder_queue")
				if hasStarcoderObserve == 1 {
					k := python.PyString_AsString(key)
					newQ := cqueue.NewCStringQueue(defaultQueueSize)
					pyQPtr := python.PyLong_FromUnsignedLongLong(newQ.GetPtr())
					result := val.CallMethodObjArgs("register_starcoder_queue", pyQPtr)
					pyQPtr.DecRef()
					if result == nil {
						newQ.Delete()
						err = errors.New(getExceptionString())
						return
					}
					observableCQueue[k] = newQ
					result.DecRef()
					s.log.Infof("found block with register_starcoder_queue: %v", k)
				}
			}
		}
	})
	return flowGraphInstance, observableCQueue, err
}

func fillDictWithParameters(dict *python.PyObject, params []*pb.StartFlowgraphRequest_Parameter) error {
	for _, param := range params {
		err := func() error {
			pyKey := python.PyString_FromString(param.GetKey())
			if pyKey == nil {
				return errors.New(getExceptionString())
			}
			defer pyKey.DecRef()
			var convertedValue *python.PyObject
			switch v := param.GetValue().GetVal().(type) {
			case *pb.Value_StringValue:
				convertedValue = python.PyString_FromString(v.StringValue)
			case *pb.Value_IntegerValue:
				convertedValue = python.PyInt_FromLong(int(v.IntegerValue))
			case *pb.Value_LongValue:
				convertedValue = python.PyLong_FromLongLong(v.LongValue)
			case *pb.Value_FloatValue:
				convertedValue = python.PyFloat_FromDouble(v.FloatValue)
			case *pb.Value_ComplexValue:
				convertedValue = python.PyComplex_FromDoubles(
					v.ComplexValue.GetRealValue(),
					v.ComplexValue.GetImaginaryValue())
			default:
				return errors.New("unsupported value type")
			}
			if convertedValue == nil {
				return errors.New(getExceptionString())
			}
			defer convertedValue.DecRef()
			err := python.PyDict_SetItem(dict, pyKey, convertedValue)
			if err != nil {
				return errors.New(getExceptionString())
			}
			return nil
		}()
		if err != nil {
			return err
		}
	}
	return nil
}

func (s *Starcoder) stopFlowGraph(flowGraphInstance *python.PyObject) error {
	var err error
	s.withPythonInterpreter(func() {
		// TODO: Check if the flow graph has already exited. Does it matter?

		stopCallReturn := flowGraphInstance.CallMethod("stop")
		if stopCallReturn == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer stopCallReturn.DecRef()
		// TODO: Is it possible "stop" won't work? Should we timeout "wait"?
		waitCallReturn := flowGraphInstance.CallMethod("wait")
		if waitCallReturn == nil {
			err = errors.New(getExceptionString())
			return
		}
		defer waitCallReturn.DecRef()
		python.PyErr_Print()
	})
	return err
}

func (s *Starcoder) withPythonInterpreter(f func()) {
	runtime.LockOSThread()
	s.gilState = python.PyGILState_Ensure()
	f()
	python.PyGILState_Release(s.gilState)
	runtime.UnlockOSThread()
}

func (s *Starcoder) Close() error {
	s.closeAllStreams()

	runtime.LockOSThread()
	s.gilState = python.PyGILState_Ensure()
	python.Finalize()
	runtime.UnlockOSThread()
	os.RemoveAll(s.tempModule)
	return nil
}

func safeAsString(obj *python.PyObject) string {
	if obj != nil {
		return python.PyString_AsString(obj)
	} else {
		return "None"
	}
}

func getExceptionString() string {
	exc, val, tb := python.PyErr_Fetch()
	if exc != nil {
		defer exc.DecRef()
	}
	if val != nil {
		defer val.DecRef()
	}
	if tb != nil {
		defer tb.DecRef()
	}
	return fmt.Sprintf("Exception: %v \n Value: %v ",
		safeAsString(exc), safeAsString(val))
}
