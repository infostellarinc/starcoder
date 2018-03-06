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
	"github.com/sbinet/go-python"
	"log"
	"runtime"
)

type PythonRuntime struct {
	funcChannel  chan func()
	closeChannel chan bool
	threadState  *python.PyThreadState
}

func NewPythonRuntime() *PythonRuntime {
	p := &PythonRuntime{
		funcChannel:  make(chan func()),
		closeChannel: make(chan bool),
	}

	go func() {
		// Lock all Python calls to a single OS thread
		runtime.LockOSThread()
		defer runtime.UnlockOSThread()
		err := python.Initialize()
		if err != nil {
			log.Fatalf("failed to initialize python: %v", err)
		}
		defer func() {
			python.PyEval_RestoreThread(p.threadState)
			python.Finalize()
		}()
		p.threadState = python.PyEval_SaveThread()
		for {
			select {
			case f := <-p.funcChannel:
				f()
			case <-p.closeChannel:
				return
			}
		}
	}()

	return p
}

func (p *PythonRuntime) Do(f func() error) error {
	done := make(chan error, 1)
	p.funcChannel <- func() {
		python.PyEval_RestoreThread(p.threadState)
		response := f()
		p.threadState = python.PyEval_SaveThread()
		done <- response
	}
	return <-done
}

func (p *PythonRuntime) Close() {
	p.closeChannel <- true
}
