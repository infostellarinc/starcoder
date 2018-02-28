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
	"os/exec"
)

type CommandRunner struct {
	command                   *exec.Cmd
	commandResponse           error
	finished                  bool
	commandResponseChannel    chan error
	isRunningRequestChannel   chan chan bool
	getCmdErrorRequestChannel chan chan error
}

func NewCommandRunner(command *exec.Cmd) *CommandRunner {
	commandRunner := &CommandRunner{
		command:                   command, // Command must already have been started
		commandResponse:           nil,
		finished:                  false,
		commandResponseChannel:    make(chan error),
		isRunningRequestChannel:   make(chan chan bool),
		getCmdErrorRequestChannel: make(chan chan error),
	}

	go func(c *CommandRunner) {
		go func() {
			c.commandResponseChannel <- c.command.Wait()
		}()

		for {
			select {
			case isRunningResponseChannel := <-c.isRunningRequestChannel:
				isRunningResponseChannel <- c.finished
			case getCommandErrorResponseChannel := <-c.getCmdErrorRequestChannel:
				getCommandErrorResponseChannel <- c.commandResponse
			case r := <-c.commandResponseChannel:
				c.commandResponse = r
				c.finished = true
			}
		}
	}(commandRunner)

	return commandRunner
}

// Returns whether a command is finished or not
func (c *CommandRunner) Finished() bool {
	responseChannel := make(chan bool)
	c.isRunningRequestChannel <- responseChannel
	return <-responseChannel
}

// Returns Cmd.Run error value. Will be nil if command
// has not yet finished, do not mistake it for a successful call
func (c *CommandRunner) GetCommandError() error {
	responseChannel := make(chan error)
	c.getCmdErrorRequestChannel <- responseChannel
	return <-responseChannel
}

func (c *CommandRunner) GetCommand() *exec.Cmd {
	// c.command is threadsafe
	return c.command
}
