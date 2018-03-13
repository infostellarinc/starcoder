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

// Sample client for testing starcoder
package main

import (
	"context"
	pb "github.com/infostellarinc/starcoder/api"
	"google.golang.org/grpc"
	"io"
	"log"
	"time"
)

func main() {
	conn, err := grpc.Dial("127.0.0.1:8990", grpc.WithInsecure())
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	defer conn.Close()
	client := pb.NewProcessManagerClient(conn)

	stream, err := client.RunFlowgraph(context.Background())
	waitc := make(chan struct{})
	go func() {
		for {
			r, err := stream.Recv()
			if err == io.EOF {
				close(waitc)
				return
			}
			if err != nil {
				log.Fatalf("%v", err)
			}
			log.Println(r.GetBlockId(), r.GetPayload())
		}
	}()
	req := &pb.RunFlowgraphRequest{
		Filename: "test.grc",
		Parameters: []*pb.RunFlowgraphRequest_Parameter{
			{
				Key: "source_filepath",
				Value: &pb.Value{
					Val: &pb.Value_StringValue{StringValue: "/home/rei/sampleAR2300IQ/full.bin"},
				},
			},
		},
	}
	if err := stream.Send(req); err != nil {
		log.Fatalf("Failed to send: %v", err)
	}
	time.Sleep(10 * time.Second)
	stream.CloseSend()
	<-waitc
}
