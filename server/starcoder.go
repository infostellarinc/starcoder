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
	pb "github.com/infostellarinc/starcoder/api"
	"golang.org/x/net/context"
)

type Starcoder struct{
	FlowgraphDir string
}

func (s *Starcoder) StartProcess(ctx context.Context, in *pb.StartProcessRequest) (*pb.StartProcessReply, error) {
	// TODO: Actually start the process
	return &pb.StartProcessReply{
		ProcessId: "1",
		Status: pb.StartProcessReply_SUCCESS,
		Error: "",
	}, nil
}

func (s *Starcoder) EndProcess(ctx context.Context, in *pb.EndProcessRequest) (*pb.EndProcessReply, error) {
	// TODO: Actually end the process
	return &pb.EndProcessReply{
		Status: pb.EndProcessReply_SUCCESS,
		Error: "",
	}, nil
}
