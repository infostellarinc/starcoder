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
	"crypto/sha1"
	"encoding/base64"
	"errors"
	"fmt"
	pb "github.com/infostellarinc/starcoder/api"
	"github.com/sbinet/go-python"
	"golang.org/x/net/context"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

type Starcoder struct {
	flowgraphDir  string
	temporaryDirs []string
	flowGraphs    map[string]*python.PyObject
	streamBlocks  map[string]map[string]*streamBlock
	threadState   *python.PyThreadState
}

type streamBlock struct {
	blockInstance          *python.PyObject
	streamClosers          map[streamCloser]bool // registered listeners
	registerStreamCloser   chan streamCloser
	deregisterStreamCloser chan streamCloser
	closeChannel           chan chan bool
}

type streamCloser = chan bool

func newStreamBlock(blockInstance *python.PyObject) *streamBlock {
	s := &streamBlock{
		blockInstance:          blockInstance,
		streamClosers:          make(map[streamCloser]bool),
		registerStreamCloser:   make(chan streamCloser),
		deregisterStreamCloser: make(chan streamCloser),
		closeChannel:           make(chan chan bool),
	}

	go func(s *streamBlock) {
		for {
			select {
			case sc := <-s.registerStreamCloser:
				s.streamClosers[sc] = true
			case sc := <-s.deregisterStreamCloser:
				delete(s.streamClosers, sc)
			case x := <-s.closeChannel:
				for sc := range s.streamClosers {
					sc <- true
				}
				x <- true
				return
			}
		}
	}(s)

	return s
}

func (s *streamBlock) Close() {
	respChannel := make(chan bool)
	s.closeChannel <- respChannel
	<-respChannel
}

func NewStarcoderServer(flowgraphDir string) *Starcoder {
	err := python.Initialize()
	if err != nil {
		log.Fatalf("failed to initialize python: %v", err)
	}
	threadState := python.PyEval_SaveThread()
	return &Starcoder{
		flowgraphDir:  flowgraphDir,
		temporaryDirs: make([]string, 0),
		flowGraphs:    make(map[string]*python.PyObject),
		streamBlocks:  make(map[string]map[string]*streamBlock),
		threadState:   threadState,
	}
}

func (s *Starcoder) StartFlowgraph(ctx context.Context, in *pb.StartFlowgraphRequest) (*pb.StartFlowgraphResponse, error) {
	inFileAbsPath := filepath.Join(s.flowgraphDir, in.GetFilename())

	if _, err := os.Stat(inFileAbsPath); os.IsNotExist(err) {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_FILE_ACCESS_ERROR,
			Error:  err.Error(),
		}, nil
	}

	var inFilePythonPath string

	if strings.HasSuffix(in.GetFilename(), ".grc") {
		grccPath, err := exec.LookPath("grcc")
		if err != nil {
			return &pb.StartFlowgraphResponse{
				Status: pb.StartFlowgraphResponse_GRC_COMPILE_ERROR,
				Error:  err.Error(),
			}, nil
		}

		// Create temporary directory to store compiled .py file.
		// We need this because we can't control the output filename
		// of the compiled file.
		// TODO: Should rename the file to something unique so it can be safely imported.
		// Weird things will happen if the module name of two different flowgraphs is the same.
		tempDir, err := ioutil.TempDir("", "starcoder")
		if err != nil {
			return &pb.StartFlowgraphResponse{
				Status: pb.StartFlowgraphResponse_GRC_COMPILE_ERROR,
				Error:  err.Error(),
			}, nil
		}
		s.temporaryDirs = append(s.temporaryDirs, tempDir)

		grccCmd := exec.Command(grccPath, "-d", tempDir, inFileAbsPath)
		err = grccCmd.Run()
		if err != nil {
			return &pb.StartFlowgraphResponse{
				Status: pb.StartFlowgraphResponse_GRC_COMPILE_ERROR,
				Error:  err.Error(),
			}, nil
		}

		files, err := ioutil.ReadDir(tempDir)
		if err != nil {
			return &pb.StartFlowgraphResponse{
				Status: pb.StartFlowgraphResponse_GRC_COMPILE_ERROR,
				Error:  err.Error(),
			}, nil
		}

		if len(files) != 1 {
			return &pb.StartFlowgraphResponse{
				Status: pb.StartFlowgraphResponse_GRC_COMPILE_ERROR,
				Error: fmt.Sprintf(
					"Unexpected number of files output by GRCC: %v", len(files)),
			}, nil
		}

		inFilePythonPath = filepath.Join(tempDir, files[0].Name())
		log.Printf("Successfully compiled GRC file to %v", inFilePythonPath)

	} else if strings.HasSuffix(in.GetFilename(), ".py") {
		inFilePythonPath = inFileAbsPath
		log.Printf("Directly using Python file at %v", inFilePythonPath)
	} else {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_UNSUPPORTED_FILE_TYPE,
			Error:  "Unsupported file type",
		}, nil
	}
	// TODO: Have some way to monitor the running process

	runtime.LockOSThread()
	python.PyEval_RestoreThread(s.threadState)
	defer func() {
		s.threadState = python.PyEval_SaveThread()
		runtime.UnlockOSThread()
	}()

	// Append module directory to sys.path
	log.Printf("Appending %v to sys.path", filepath.Dir(inFilePythonPath))
	sysPath := python.PySys_GetObject("path")
	if sysPath == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	moduleDir := python.PyString_FromString(filepath.Dir(inFilePythonPath))
	if moduleDir == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer moduleDir.DecRef()
	err := python.PyList_Append(sysPath, moduleDir)
	if err != nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  err.Error(),
		}, nil
	}

	emptyTuple := python.PyTuple_New(0)
	if emptyTuple == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer emptyTuple.DecRef()

	// Import module
	moduleName := strings.TrimSuffix(filepath.Base(inFilePythonPath), filepath.Ext(filepath.Base(inFilePythonPath)))
	log.Printf("Importing %v", moduleName)
	module := python.PyImport_ImportModule(moduleName)
	if module == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer module.DecRef()

	// Find top_block subclass
	// GRC compiled python scripts have the top block class name equal to the python filename.
	flowGraphClassName := moduleName
	flowgraphClass := module.GetAttrString(flowGraphClassName)
	if flowgraphClass == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer flowgraphClass.DecRef()
	gnuRadioModule := python.PyImport_ImportModule("gnuradio")
	if gnuRadioModule == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer gnuRadioModule.DecRef()
	grModule := gnuRadioModule.GetAttrString("gr")
	if grModule == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer grModule.DecRef()
	topBlock := grModule.GetAttrString("top_block")
	if topBlock == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer topBlock.DecRef()

	// Verify top_block subclass
	isSubclass := flowgraphClass.IsSubclass(topBlock)
	if isSubclass == 0 {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error: fmt.Sprintf(
				"Top block class %v is not a "+
					"subclass of gnuradio.gr.top_block", flowGraphClassName),
		}, nil
	} else if isSubclass == -1 {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}

	kwArgs := python.PyDict_New()
	if kwArgs == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer kwArgs.DecRef()

	for _, param := range in.GetParameters() {
		response := func() *pb.StartFlowgraphResponse {
			pyKey := python.PyString_FromString(param.GetKey())
			if pyKey == nil {
				return &pb.StartFlowgraphResponse{
					Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
					Error:  getExceptionString(),
				}
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
				return &pb.StartFlowgraphResponse{
					Status: pb.StartFlowgraphResponse_UNKNOWN_ERROR,
					Error:  "Unsupported value type",
				}
			}
			if convertedValue == nil {
				return &pb.StartFlowgraphResponse{
					Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
					Error:  getExceptionString(),
				}
			}
			defer convertedValue.DecRef()
			err = python.PyDict_SetItem(kwArgs, pyKey, convertedValue)
			if err != nil {
				return &pb.StartFlowgraphResponse{
					Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
					Error:  err.Error(),
				}
			}
			return nil
		}()
		if response != nil {
			return response, nil
		}
	}

	flowGraphInstance := flowgraphClass.Call(emptyTuple, kwArgs)
	if flowGraphInstance == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}

	callReturn := flowGraphInstance.CallMethod("start")
	if callReturn == nil {
		return &pb.StartFlowgraphResponse{
			Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer callReturn.DecRef()

	hash := sha1.Sum([]byte(fmt.Sprintf("%v", flowGraphInstance.GetCPointer())))
	uniqueId := base64.URLEncoding.EncodeToString(hash[:])
	uniqueId = "1"

	s.flowGraphs[uniqueId] = flowGraphInstance
	s.streamBlocks[uniqueId] = make(map[string]*streamBlock)

	// Look for any Enqueue Message Sink blocks
	starcoderModule := python.PyImport_ImportModule("starcoder")
	if starcoderModule == nil {
		log.Println("gr-starcoder module not found. There are no Enqueue Message Sink blocks")
	} else {
		defer starcoderModule.DecRef()

		enqueueMessageSink := starcoderModule.GetAttrString("enqueue_msg_sink")
		if enqueueMessageSink == nil {
			log.Println("gr-starcoder module does not contain enqueue_msg_sink")
		} else {
			defer enqueueMessageSink.DecRef()
			flowGraphDict := flowGraphInstance.GetAttrString("__dict__")
			if flowGraphDict == nil {
				return &pb.StartFlowgraphResponse{
					Status: pb.StartFlowgraphResponse_PYTHON_RUN_ERROR,
					Error:  getExceptionString(),
				}, nil
			}
			defer flowGraphDict.DecRef()

			iter := 0
			key := python.PyString_FromString("")
			val := python.PyString_FromString("")
			for python.PyDict_Next(flowGraphDict, &iter, &key, &val) {
				// Verify enqueue_msg_sink instance
				isInstance := val.IsInstance(enqueueMessageSink)
				if isInstance == 1 {
					k := python.PyString_AsString(key)
					s.streamBlocks[uniqueId][k] = newStreamBlock(val)
					fmt.Println("found enqueue_msg_sink block:", k)
				} else if isInstance == -1 {
					log.Println(getExceptionString())
				}
			}
		}
	}

	return &pb.StartFlowgraphResponse{
		ProcessId: uniqueId,
		Status:    pb.StartFlowgraphResponse_SUCCESS,
		Error:     "",
	}, nil
}

func (s *Starcoder) StreamPmt(request *pb.StreamPmtRequest, stream pb.ProcessManager_StreamPmtServer) error {
	var flowGraphStreamBlocks map[string]*streamBlock
	if sbs, ok := s.streamBlocks[request.GetProcessId()]; !ok {
		return errors.New(fmt.Sprintf("Invalid flowgraph ID %v", request.GetProcessId()))
	} else {
		flowGraphStreamBlocks = sbs
	}
	var streamBlock *streamBlock
	if sb, ok := flowGraphStreamBlocks[request.GetBlockId()]; !ok {
		return errors.New(fmt.Sprintf("Invalid block ID %v", request.GetBlockId()))
	} else {
		streamBlock = sb
	}

	sc := make(chan bool)
	streamBlock.registerStreamCloser <- sc

	python.PyEval_RestoreThread(s.threadState)
	pmtQueue := streamBlock.blockInstance.CallMethod("observe")
	if pmtQueue == nil {
		return errors.New(getExceptionString())
	}
	s.threadState = python.PyEval_SaveThread()
	defer func() {
		python.PyEval_RestoreThread(s.threadState)
		pmtQueue.DecRef()
		s.threadState = python.PyEval_SaveThread()
	}()

	ticker := time.NewTicker(time.Millisecond * 100) // Poll the PMT queue every 100ms
	for {
		select {
		case <-ticker.C:
			err := func() error {
				runtime.LockOSThread()
				python.PyEval_RestoreThread(s.threadState)
				defer func() {
					s.threadState = python.PyEval_SaveThread()
					runtime.UnlockOSThread()
				}()
				pyLength := pmtQueue.CallMethod("__len__")
				length := python.PyInt_AsLong(pyLength)

				emptyTuple := python.PyTuple_New(0)
				if emptyTuple == nil {
					return errors.New(getExceptionString())
				}
				defer emptyTuple.DecRef()

				for i := 0; i < length; i++ {
					pmt := pmtQueue.CallMethod("popleft")
					if pmt == nil {
						return errors.New(getExceptionString())
					}
					// TODO: Convert PMT to a gRPC native data structure.
					// Use built-in PMT serialization for now.
					pmtBytes := python.PyByteArray_AsBytes(pmt)
					pmt.DecRef()
					log.Println(pmtBytes)
					if err := stream.Send(&pb.StreamPmtResponse{Bytes: []byte(pmtBytes)}); err != nil {
						return err
					}
				}
				return nil
			}()
			if err != nil {
				// Only need to deregister if the stream is broken through error
				streamBlock.deregisterStreamCloser <- sc
				return err
			}
		case <-sc:
			return nil
		}
	}
	return nil
}

func (s *Starcoder) EndFlowgraph(ctx context.Context, in *pb.EndFlowgraphRequest) (*pb.EndFlowgraphResponse, error) {
	if _, ok := s.flowGraphs[in.GetProcessId()]; !ok {
		return &pb.EndFlowgraphResponse{
			Status: pb.EndFlowgraphResponse_INVALID_PROCESS_ID,
			Error:  fmt.Sprintf("Invalid process ID %v", in.GetProcessId()),
		}, nil
	}

	sbs := s.streamBlocks[in.GetProcessId()]
	for _, sb := range sbs {
		sb.Close()
	}

	runtime.LockOSThread()
	python.PyEval_RestoreThread(s.threadState)
	defer func() {
		s.threadState = python.PyEval_SaveThread()
		runtime.UnlockOSThread()
	}()

	for _, sb := range sbs {
		sb.blockInstance.DecRef()
	}

	flowGraph := s.flowGraphs[in.GetProcessId()]

	// TODO: Check if the flow graph has already exited. Does it matter?

	stopCallReturn := flowGraph.CallMethod("stop")
	if stopCallReturn == nil {
		return &pb.EndFlowgraphResponse{
			Status: pb.EndFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer stopCallReturn.DecRef()
	// TODO: Is it possible "stop" won't work? Should we timeout "wait"?
	waitCallReturn := flowGraph.CallMethod("wait")
	if waitCallReturn == nil {
		return &pb.EndFlowgraphResponse{
			Status: pb.EndFlowgraphResponse_PYTHON_RUN_ERROR,
			Error:  getExceptionString(),
		}, nil
	}
	defer waitCallReturn.DecRef()
	python.PyErr_Print()
	flowGraph.DecRef()
	delete(s.streamBlocks, in.GetProcessId())
	delete(s.flowGraphs, in.GetProcessId())

	return &pb.EndFlowgraphResponse{
		Status: pb.EndFlowgraphResponse_SUCCESS,
	}, nil
}

func (s *Starcoder) Close() error {
	for _, sbs := range s.streamBlocks {
		for _, sb := range sbs {
			sb.Close()
		}
	}

	runtime.LockOSThread()
	python.PyEval_RestoreThread(s.threadState)
	defer func() {
		python.Finalize()
		runtime.UnlockOSThread()
	}()
	for _, flowGraph := range s.flowGraphs {
		stopCallReturn := flowGraph.CallMethod("stop")
		if stopCallReturn == nil {
			log.Println(getExceptionString())
		} else {
			stopCallReturn.DecRef()
		}
		// TODO: Is it possible "stop" won't work? Should we timeout wait?
		waitCallReturn := flowGraph.CallMethod("wait")
		if waitCallReturn == nil {
			log.Println(getExceptionString())
		} else {
			waitCallReturn.DecRef()
		}
		python.PyErr_Print()
		flowGraph.DecRef()
	}
	for _, tempDir := range s.temporaryDirs {
		os.RemoveAll(tempDir)
	}
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
