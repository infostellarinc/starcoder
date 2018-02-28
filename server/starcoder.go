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
	"golang.org/x/net/context"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
)

type Starcoder struct {
	flowgraphDir  string
	temporaryDirs []string
	processes     map[string]*os.Process
}

func NewStarcoderServer(flowgraphDir string) *Starcoder {
	return &Starcoder{
		flowgraphDir:  flowgraphDir,
		temporaryDirs: make([]string, 0),
		processes:     make(map[string]*os.Process),
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
	gnuRadioCmd := exec.Command("python", cliParameters...)
	err := gnuRadioCmd.Start()
	if err != nil {
		return &pb.StartProcessReply{
			Status: pb.StartProcessReply_PYTHON_RUN_ERROR,
			Error:  err.Error(),
		}, nil
	}
	log.Printf("Ran command python with parameters %v", cliParameters)

	process := gnuRadioCmd.Process
	s.processes[strconv.Itoa(process.Pid)] = process
	// TODO: Have some way to monitor the running process

	return &pb.StartProcessReply{
		ProcessId: strconv.Itoa(process.Pid),
		Status:    pb.StartProcessReply_SUCCESS,
		Error:     "",
	}, nil
}

func (s *Starcoder) EndProcess(ctx context.Context, in *pb.EndProcessRequest) (*pb.EndProcessReply, error) {
	if _, ok := s.processes[in.GetProcessId()]; !ok {
		return &pb.EndProcessReply{
			Status: pb.EndProcessReply_INVALID_PROCESS_ID,
			Error:  fmt.Sprintf("Invalid process ID %v", in.GetProcessId()),
		}, nil
	}

	proc := s.processes[in.GetProcessId()]

	// TODO: Check if process is even still running at this point
	err := proc.Signal(os.Interrupt)
	if err != nil {
		return &pb.EndProcessReply{
			Status: pb.EndProcessReply_UNKNOWN_ERROR,
			Error:  err.Error(),
		}, nil
	}

	// TODO: Handle processes that won't end with SIGINT, as this will wait forever
	_, err = proc.Wait()
	if err != nil {
		return &pb.EndProcessReply{
			Status: pb.EndProcessReply_UNKNOWN_ERROR,
			Error:  err.Error(),
		}, nil
	}

	delete(s.processes, in.GetProcessId())

	return &pb.EndProcessReply{
		Status: pb.EndProcessReply_SUCCESS,
		// TODO: Send statistics of the ended process through its ProcessState
	}, nil
}

func (s *Starcoder) Close() error {
	for _, process := range s.processes {
		// TODO: Do something gentler than immediately killing?
		process.Kill()
	}
	for _, tempDir := range s.temporaryDirs {
		os.RemoveAll(tempDir)
	}
	return nil
}
