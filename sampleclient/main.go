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
	"io/ioutil"
	"log"
	"time"
)

func main() {
	conn, err := grpc.Dial("127.0.0.1:8990", grpc.WithInsecure())
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	defer conn.Close()
	client := pb.NewStarcoderClient(conn)

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
				log.Println("error receiving!")
				log.Fatalf("%v", err)
			}
			if len(r.GetPayload()) > 20 {
				log.Println(r.GetBlockId(), len(r.GetPayload()))
			} else {
				log.Println(r.GetBlockId(), r.GetPayload())
			}
			if r.GetBlockId() == "starcoder_waterfall_sink_0" {
				ioutil.WriteFile("/home/rei/sampleAR2300IQ/waterfall_rec.png", r.GetPayload(), 0644)
			}
		}
	}()
	startReq := &pb.StartFlowgraphRequest{
		Filename: "test.grc",
	}
	req := &pb.RunFlowgraphRequest{
		Request: &pb.RunFlowgraphRequest_StartFlowgraphRequest{
			StartFlowgraphRequest: startReq,
		},
	}
	if err := stream.Send(req); err != nil {
		log.Fatalf("Failed to send: %v", err)
	}
	time.Sleep(1 * time.Second)
	commandReq := &pb.SendCommandRequest{
		BlockId: "starcoder_command_source_0",
		Pmt: constructPDU(),
	}
	req = &pb.RunFlowgraphRequest{
		Request: &pb.RunFlowgraphRequest_SendCommandRequest{
			SendCommandRequest: commandReq,
		},
	}
	if err := stream.Send(req); err != nil {
		log.Fatalf("Failed to send: %v", err)
	}
	time.Sleep(9 * time.Second)
	stream.CloseSend()
	<-waitc
}

func constructPDU() *pb.BlockMessage {
	pmtDict := &pb.Dict{
		Entry: []*pb.Dict_Entry{
			{
				Key: &pb.BlockMessage{
					MessageOneof: &pb.BlockMessage_SymbolValue{"metadata1"},
				},
				Value: &pb.BlockMessage{
					MessageOneof: &pb.BlockMessage_IntegerValue{1},
				},
			},
		},
	}
	pmtPair := &pb.Pair{
		Car: &pb.BlockMessage{
			MessageOneof: &pb.BlockMessage_DictValue{pmtDict},
		},
		Cdr: constructU8Vector(),
	}
	return &pb.BlockMessage{
		MessageOneof: &pb.BlockMessage_PairValue{pmtPair},
	}
}

func constructU8Vector() *pb.BlockMessage {
	pmtUVector := &pb.UniformVector{
		UniformVectorOneof: &pb.UniformVector_UValue{
			UValue: &pb.UVector{
				Value: []uint32{117, 105, 110, 116, 56, 20, 112, 97, 121, 108, 111, 97, 100},
				Size_: pb.IntSize_Size8,
			},
		},
	}
	return &pb.BlockMessage{
		MessageOneof: &pb.BlockMessage_UniformVectorValue{pmtUVector},
	}
}
