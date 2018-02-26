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
 *
 */

//go:generate protoc -I ./starcoder-proto/src/main/proto --go_out=plugins=grpc:./protobuf ./starcoder-proto/src/main/proto/starcoder.proto

package main

import (
	pb "github.com/infostellarinc/starcoder/protobuf"
	"golang.org/x/net/context"
	"net"
	"log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"
	"os/user"
	"path/filepath"
)

const (
	// TODO: Replace this with command line argument
	bindAddress = ":50051"
)

type server struct{
	flowgraphDir string
}

func (s *server) StartProcess(ctx context.Context, in *pb.StartProcessRequest) (*pb.StartProcessReply, error) {
	return &pb.StartProcessReply{
		ProcessId: "1",
		Status: pb.StartProcessReply_SUCCESS,
		Error: "",
	}, nil
}

func (s *server) EndProcess(ctx context.Context, in *pb.EndProcessRequest) (*pb.EndProcessReply, error) {
	return &pb.EndProcessReply{
		Status: pb.EndProcessReply_SUCCESS,
		Error: "",
	}, nil
}

func main() {
	usr, err := user.Current()
	if err != nil {
		log.Fatalf("failed to get user: %v", err)
	}
	lis, err := net.Listen("tcp", bindAddress)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	pb.RegisterProcessManagerServer(s, &server{
		// TODO: Replace this with command line argument
		flowgraphDir: filepath.Join(usr.HomeDir, ".starcoder/flowgraphs"),
	})
	// Register reflection service on gRPC server.
	reflection.Register(s)
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
