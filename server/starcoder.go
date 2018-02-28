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
	"time"
)

type Starcoder struct {
	flowgraphDir  string
	temporaryDirs []string
	commands      map[string]*exec.Cmd
}

func NewStarcoderServer(flowgraphDir string) *Starcoder {
	return &Starcoder{
		flowgraphDir:  flowgraphDir,
		temporaryDirs: make([]string, 0),
		commands:      make(map[string]*exec.Cmd),
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
	log.Printf("Executing: %s\n", strings.Join(gnuRadioCmd.Args, " "))
	go gnuRadioCmd.Wait()

	s.commands[strconv.Itoa(gnuRadioCmd.Process.Pid)] = gnuRadioCmd
	// TODO: Have some way to monitor the running process

	return &pb.StartProcessReply{
		ProcessId: strconv.Itoa(gnuRadioCmd.Process.Pid),
		Status:    pb.StartProcessReply_SUCCESS,
		Error:     "",
	}, nil
}

func (s *Starcoder) EndProcess(ctx context.Context, in *pb.EndProcessRequest) (*pb.EndProcessReply, error) {
	if _, ok := s.commands[in.GetProcessId()]; !ok {
		return &pb.EndProcessReply{
			Status: pb.EndProcessReply_INVALID_PROCESS_ID,
			Error:  fmt.Sprintf("Invalid process ID %v", in.GetProcessId()),
		}, nil
	}

	cmd := s.commands[in.GetProcessId()]

	if cmd.ProcessState != nil {
		return &pb.EndProcessReply{
			Status: pb.EndProcessReply_PROCESS_EXITED,
			Error:  "Process has already exited",
		}, nil
	}

	err := cmd.Process.Signal(os.Interrupt)
	if err != nil {
		return &pb.EndProcessReply{
			Status: pb.EndProcessReply_UNKNOWN_ERROR,
			Error:  err.Error(),
		}, nil
	}

	ticker := time.NewTicker(time.Second)
	defer ticker.Stop()
	secsPassed := 0
	for breakOut := false; breakOut == false; {
		select {
		case <-ticker.C:
			secsPassed += 1
			if cmd.ProcessState != nil || secsPassed >= int(in.GetTimeout()) {
				breakOut = true
			}
		}
	}

	if cmd.ProcessState == nil {
		err := cmd.Process.Signal(os.Kill)
		if err != nil {
			return &pb.EndProcessReply{
				Status: pb.EndProcessReply_UNKNOWN_ERROR,
				Error:  err.Error(),
			}, nil
		}
		for cmd.ProcessState == nil {
			time.Sleep(time.Millisecond)
		}
	}

	return &pb.EndProcessReply{
		Status: pb.EndProcessReply_SUCCESS,
		// TODO: Send statistics of the ended process through its ProcessState
	}, nil
}

func (s *Starcoder) Close() error {
	for _, cmd := range s.commands {
		// TODO: Do something gentler than immediately killing?
		cmd.Process.Kill()
	}
	for _, tempDir := range s.temporaryDirs {
		os.RemoveAll(tempDir)
	}
	return nil
}
