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
	"fmt"
	pb "github.com/infostellarinc/starcoder/api"
	"github.com/infostellarinc/starcoder/util"
	"github.com/sbinet/go-python"
	"golang.org/x/net/context"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

type Starcoder struct {
	flowgraphDir  string
	temporaryDirs []string
	flowGraphs    map[string]*python.PyObject
}

func NewStarcoderServer(flowgraphDir string) *Starcoder {
	return &Starcoder{
		flowgraphDir:  flowgraphDir,
		temporaryDirs: make([]string, 0),
		flowGraphs:    make(map[string]*python.PyObject),
	}
}

func (s *Starcoder) StartProcess(ctx context.Context, in *pb.StartProcessRequest) (*pb.StartProcessReply, error) {
	inFileAbsPath := filepath.Join(s.flowgraphDir, in.GetFilename())

	if _, err := os.Stat(inFileAbsPath); os.IsNotExist(err) {
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_FILE_ACCESS_ERROR,
			Error:  err.Error(),
		}, nil
	}

	var inFilePythonPath string

	if strings.HasSuffix(in.GetFilename(), ".grc") {
		grccPath, err := exec.LookPath("grcc")
		if err != nil {
			return &pb.StartProcessReply{
				Status: pb.StartProcessReply_GRC_COMPILE_ERROR,
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
			return &pb.StartProcessReply{
				Status: pb.StartProcessReply_GRC_COMPILE_ERROR,
				Error:  err.Error(),
			}, nil
		}
		s.temporaryDirs = append(s.temporaryDirs, tempDir)

		grccCmd := exec.Command(grccPath, "-d", tempDir, inFileAbsPath)
		err = grccCmd.Run()
		if err != nil {
			return &pb.StartProcessReply{
				Status: pb.StartProcessReply_GRC_COMPILE_ERROR,
				Error:  err.Error(),
			}, nil
		}

		files, err := ioutil.ReadDir(tempDir)
		if err != nil {
			return &pb.StartProcessReply{
				Status: pb.StartProcessReply_GRC_COMPILE_ERROR,
				Error:  err.Error(),
			}, nil
		}

		if len(files) != 1 {
			return &pb.StartProcessReply{
				Status: pb.StartProcessReply_GRC_COMPILE_ERROR,
				Error:  fmt.Sprintf("Unexpected number of files output by GRCC: %v", len(files)),
			}, nil
		}

		inFilePythonPath = filepath.Join(tempDir, files[0].Name())
		log.Printf("Successfully compiled GRC file to %v", inFilePythonPath)

	} else if strings.HasSuffix(in.GetFilename(), ".py") {
		inFilePythonPath = inFileAbsPath
		log.Printf("Directly using Python file at %v", inFilePythonPath)
	} else {
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_UNSUPPORTED_FILE_TYPE,
			Error:  "Unsupported file type",
		}, nil
	}

	var cliParameters []string
	cliParameters = append(cliParameters, inFilePythonPath)
	for _, param := range in.GetParameters() {
		cliParameters = append(cliParameters, "--"+param.GetKey(), param.GetValue())
	}
	// TODO: Have some way to monitor the running process

	sysPath := python.PySys_GetObject("path")
	moduleDir := python.PyString_FromString(filepath.Dir(inFilePythonPath))
	log.Println(filepath.Dir(inFilePythonPath))
	err := python.PyList_Append(sysPath, moduleDir)
	if err != nil {
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_PYTHON_RUN_ERROR,
			Error:  err.Error(),
		}, nil
	}
	moduleDir.DecRef()
	moduleName := strings.TrimSuffix(filepath.Base(inFilePythonPath), filepath.Ext(filepath.Base(inFilePythonPath)))
	log.Printf("Importing %v", moduleName)
	module := python.PyImport_ImportModule(moduleName)
	if module == nil {
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_PYTHON_RUN_ERROR,
			Error:  fmt.Sprintf("Module %v failed to load", moduleName),
		}, nil
	}
	// GRC compiled python scripts have the top block class name equal to the python filename.
	flowGraphClassName := moduleName
	flowgraphClass := module.GetAttrString(flowGraphClassName)
	if flowgraphClass == nil {
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_PYTHON_RUN_ERROR,
			Error:  fmt.Sprintf("Top block class %v not found", flowGraphClassName),
		}, nil
	}
	gnuRadioModule := python.PyImport_ImportModule("gnuradio")
	grModule := gnuRadioModule.GetAttrString("gr")
	gnuRadioModule.DecRef()
	topBlock := grModule.GetAttrString("top_block")
	grModule.DecRef()

	if flowgraphClass.IsSubclass(topBlock) != 1 {
		// TODO: Handle error when return value is -1
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_PYTHON_RUN_ERROR,
			Error:  fmt.Sprintf("Top block class %v is not a subclass of gnuradio.gr.top_block", flowGraphClassName),
		}, nil
	}
	topBlock.DecRef()

	// TODO: Instantiate flow graph with passed parameters, if any
	flowGraphInstance := flowgraphClass.CallObject(python.PyTuple_New(0))
	flowgraphClass.DecRef()
	if flowGraphInstance == nil {
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_PYTHON_RUN_ERROR,
			Error:  "Could not instantiate flow graph class",
		}, nil
	}

	// TODO: Check and handle errors on starting flowgraph
	flowGraphInstance.CallMethod("start")
	python.PyErr_Print()

	var uniqueId string
	for ok := true; ok; _, ok = s.flowGraphs[uniqueId] {
		uniqueId = util.RandString(4)
	}

	s.flowGraphs[uniqueId] = flowGraphInstance

	return &pb.StartProcessReply{
		ProcessId: uniqueId,
		Status:    pb.StartProcessReply_SUCCESS,
		Error:     "",
	}, nil
}

func (s *Starcoder) EndProcess(ctx context.Context, in *pb.EndProcessRequest) (*pb.EndProcessReply, error) {
	if _, ok := s.flowGraphs[in.GetProcessId()]; !ok {
		return &pb.EndProcessReply{
			Status: pb.EndProcessReply_INVALID_PROCESS_ID,
			Error:  fmt.Sprintf("Invalid process ID %v", in.GetProcessId()),
		}, nil
	}

	flowGraph := s.flowGraphs[in.GetProcessId()]

	// TODO: Check if the flow graph has already exited. Does it matter?

	flowGraph.CallMethod("stop")
	// TODO: Is it possible "stop" won't work? Should we timeout "wait"?
	flowGraph.CallMethod("wait")
	// TODO: Check for errors after calling stop and wait.
	python.PyErr_Print()
	flowGraph.DecRef()
	delete(s.flowGraphs, in.GetProcessId())

	return &pb.EndProcessReply{
		Status: pb.EndProcessReply_SUCCESS,
	}, nil
}

func (s *Starcoder) Close() error {
	for _, flowGraph := range s.flowGraphs {
		flowGraph.CallMethod("stop")
		// TODO: Is it possible "stop" won't work? Should we timeout wait?
		flowGraph.CallMethod("wait")
		// TODO: Check for errors after calling stop and wait.
		python.PyErr_Print()
		flowGraph.DecRef()
	}
	for _, tempDir := range s.temporaryDirs {
		os.RemoveAll(tempDir)
	}
	return nil
}
