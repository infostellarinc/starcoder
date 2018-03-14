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
	"github.com/sbinet/go-python"
	"io"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

import "C"

type Starcoder struct {
	flowgraphDir  string
	temporaryDirs []string
	threadState   *python.PyThreadState
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
		threadState:   threadState,
	}
}

func (s *Starcoder) compileGrc(path string) (string, error) {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return "", err
	}

	var inFilePythonPath string

	if strings.HasSuffix(path, ".grc") {
		grccPath, err := exec.LookPath("grcc")
		if err != nil {
			return "", err
		}

		// Create temporary directory to store compiled .py file.
		// We need this because we can't control the output filename
		// of the compiled file.
		// TODO: Should rename the file to something unique so it can be safely imported.
		// Weird things will happen if the module name of two different flowgraphs is the same.
		tempDir, err := ioutil.TempDir("", "starcoder")
		if err != nil {
			return "", err
		}
		s.temporaryDirs = append(s.temporaryDirs, tempDir)

		grccCmd := exec.Command(grccPath, "-d", tempDir, path)
		err = grccCmd.Run()
		if err != nil {
			return "", err
		}

		files, err := ioutil.ReadDir(tempDir)
		if err != nil {
			return "", err
		}

		if len(files) != 1 {
			return "", errors.New(fmt.Sprintf(
				"Unexpected number of files output by GRCC: %v", len(files)))
		}

		inFilePythonPath = filepath.Join(tempDir, files[0].Name())
		log.Printf("Successfully compiled GRC file to %v", inFilePythonPath)

	} else if strings.HasSuffix(path, ".py") {
		inFilePythonPath = path
		log.Printf("Directly using Python file at %v", inFilePythonPath)
	} else {
		return "", errors.New("unsupported file type")
	}
	return inFilePythonPath, nil
}

func (s *Starcoder) RunFlowgraph(stream pb.Starcoder_RunFlowgraphServer) error {
	in, err := stream.Recv()
	if err == io.EOF {
		return nil
	}
	if err != nil {
		return err
	}

	inFileAbsPath := filepath.Join(s.flowgraphDir, in.GetFilename())

	inFilePythonPath, err := s.compileGrc(inFileAbsPath)
	if err != nil {
		return err
	}

	flowGraphInstance, msgSinkBlocks, err := func() (*python.PyObject, map[string]*python.PyObject, error) {
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
			return nil, nil, errors.New(getExceptionString())
		}
		moduleDir := python.PyString_FromString(filepath.Dir(inFilePythonPath))
		if moduleDir == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer moduleDir.DecRef()
		err = python.PyList_Append(sysPath, moduleDir)
		if err != nil {
			return nil, nil, err
		}

		// Import module
		moduleName := strings.TrimSuffix(filepath.Base(inFilePythonPath), filepath.Ext(filepath.Base(inFilePythonPath)))
		log.Printf("Importing %v", moduleName)
		module := python.PyImport_ImportModule(moduleName)
		if module == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer module.DecRef()

		// Find top_block subclass
		// GRC compiled python scripts have the top block class name equal to the python filename.
		flowGraphClassName := moduleName
		flowgraphClass := module.GetAttrString(flowGraphClassName)
		if flowgraphClass == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer flowgraphClass.DecRef()
		gnuRadioModule := python.PyImport_ImportModule("gnuradio")
		if gnuRadioModule == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer gnuRadioModule.DecRef()
		grModule := gnuRadioModule.GetAttrString("gr")
		if grModule == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer grModule.DecRef()
		topBlock := grModule.GetAttrString("top_block")
		if topBlock == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer topBlock.DecRef()

		// Verify top_block subclass
		isSubclass := flowgraphClass.IsSubclass(topBlock)
		if isSubclass == 0 {
			return nil, nil, errors.New(fmt.Sprintf(
				"Top block class %v is not a "+
					"subclass of gnuradio.gr.top_block", flowGraphClassName))

		} else if isSubclass == -1 {
			return nil, nil, errors.New(getExceptionString())
		}

		kwArgs := python.PyDict_New()
		if kwArgs == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer kwArgs.DecRef()

		for _, param := range in.GetParameters() {
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
				err = python.PyDict_SetItem(kwArgs, pyKey, convertedValue)
				if err != nil {
					return errors.New(getExceptionString())
				}
				return nil
			}()
			if err != nil {
				return nil, nil, err
			}
		}

		emptyTuple := python.PyTuple_New(0)
		if emptyTuple == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer emptyTuple.DecRef()

		flowGraphInstance := flowgraphClass.Call(emptyTuple, kwArgs)
		if flowGraphInstance == nil {
			return nil, nil, errors.New(getExceptionString())
		}

		callReturn := flowGraphInstance.CallMethod("start")
		if callReturn == nil {
			return nil, nil, errors.New(getExceptionString())
		}
		defer callReturn.DecRef()

		msgSinkBlocks := make(map[string]*python.PyObject)

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
					return nil, nil, errors.New(getExceptionString())
				}
				defer flowGraphDict.DecRef()

				iter := 0
				key := python.PyString_FromString("")
				defer key.DecRef()
				val := python.PyString_FromString("")
				defer val.DecRef()
				for python.PyDict_Next(flowGraphDict, &iter, &key, &val) {
					// Verify enqueue_msg_sink instance
					isInstance := val.IsInstance(enqueueMessageSink)
					if isInstance == 1 {
						k := python.PyString_AsString(key)
						msgSinkBlocks[k] = val
						fmt.Println("found enqueue_msg_sink block:", k)
					} else if isInstance == -1 {
						log.Println(getExceptionString())
					}
				}
			}
		}
		return flowGraphInstance, msgSinkBlocks, nil
	}()
	if err != nil {
		return err
	}

	pmtQueues := make(map[string]*python.PyObject)

	for key, b := range msgSinkBlocks {
		python.PyEval_RestoreThread(s.threadState)
		pmtQueue := b.CallMethod("observe")
		if pmtQueue == nil {
			return errors.New(getExceptionString())
		}
		pmtQueues[key] = pmtQueue
		s.threadState = python.PyEval_SaveThread()
	}
	defer func() {
		python.PyEval_RestoreThread(s.threadState)
		for _, blockInstance := range msgSinkBlocks {
			blockInstance.DecRef()
		}
		s.threadState = python.PyEval_SaveThread()
	}()

	closeChannel := make(chan bool)
	ticker := time.NewTicker(time.Millisecond * 100)

	go func() {
		for {
			// Beyond the first packet, we don't care what the client
			// is sending us until it wants to end the connection.
			_, err := stream.Recv()
			if err == io.EOF {
				// Client is done listening
				closeChannel <- true
				return
			}
			if err != nil {
				closeChannel <- true
				log.Printf("%v", err)
				return
			}
		}
	}()

	// Streaming loop here
	err = func() error {
		for {
			select {
			case <-closeChannel:
				err := s.stopFlowGraph(flowGraphInstance)
				if err != nil {
					return err
				}
				for k, pmtQueue := range pmtQueues {
					bytes, err := s.getBytesFromQueue(pmtQueue)
					if err != nil {
						return err
					}
					for _, b := range bytes {
						if err := stream.Send(&pb.RunFlowgraphResponse{
							BlockId: k,
							Payload: b,
						}); err != nil {
							return err
						}
					}
				}
				return nil
			case <-ticker.C:
				for k, pmtQueue := range pmtQueues {
					bytes, err := s.getBytesFromQueue(pmtQueue)
					if err != nil {
						return err
					}
					for _, b := range bytes {
						if err := stream.Send(&pb.RunFlowgraphResponse{
							BlockId: k,
							Payload: b,
						}); err != nil {
							return err
						}
					}
				}
			}
		}
	}()
	if err != nil {
		return err
	}

	return nil
}

func (s *Starcoder) getBytesFromQueue(pmtQueue *python.PyObject) ([][]byte, error) {
	runtime.LockOSThread()
	python.PyEval_RestoreThread(s.threadState)
	defer func() {
		s.threadState = python.PyEval_SaveThread()
		runtime.UnlockOSThread()
	}()
	length := python.PyList_Size(pmtQueue)

	var bytes [][]byte

	emptyList := python.PyList_New(0)
	defer emptyList.DecRef()

	for i := 0; i < length; i++ {
		// TODO: Convert PMT to a gRPC native data structure.
		// Use built-in PMT serialization for now.
		pmtBytes := python.PyByteArray_AsBytes(python.PyList_GetItem(pmtQueue, i))
		err := python.PyList_SetSlice(pmtQueue, 0, length, emptyList)
		if err != nil {
			return nil, err
		}
		bytes = append(bytes, pmtBytes)
	}
	return bytes, nil
}

func (s *Starcoder) stopFlowGraph(flowGraphInstance *python.PyObject) error {
	runtime.LockOSThread()
	python.PyEval_RestoreThread(s.threadState)
	defer func() {
		s.threadState = python.PyEval_SaveThread()
		runtime.UnlockOSThread()
	}()

	// TODO: Check if the flow graph has already exited. Does it matter?

	stopCallReturn := flowGraphInstance.CallMethod("stop")
	if stopCallReturn == nil {
		return errors.New(getExceptionString())
	}
	defer stopCallReturn.DecRef()
	// TODO: Is it possible "stop" won't work? Should we timeout "wait"?
	waitCallReturn := flowGraphInstance.CallMethod("wait")
	if waitCallReturn == nil {
		return errors.New(getExceptionString())
	}
	defer waitCallReturn.DecRef()
	python.PyErr_Print()
	flowGraphInstance.DecRef()
	return nil
}

func (s *Starcoder) Close() error {

	runtime.LockOSThread()
	python.PyEval_RestoreThread(s.threadState)
	defer func() {
		python.Finalize()
		runtime.UnlockOSThread()
	}()
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
