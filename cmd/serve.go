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

package cmd

import (
	"github.com/spf13/cobra"
	pb "github.com/infostellarinc/starcoder/api"
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

// serveCmd represents the serve command
var serveCmd = &cobra.Command{
	Use:   "serve",
	Short: "Start the Starcoder server",
	Long: `Start the Starcoder server`,
	Run: func(cmd *cobra.Command, args []string) {
		log.Println("serve called")
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
			// TODO: Replace this with command line argument and use Packr to default to compiled in flowgraphs
			flowgraphDir: filepath.Join(usr.HomeDir, ".starcoder/flowgraphs"),
		})
		// Register reflection service on gRPC server.
		reflection.Register(s)
		if err := s.Serve(lis); err != nil {
			log.Fatalf("failed to serve: %v", err)
		}
	},
}

func init() {
	rootCmd.AddCommand(serveCmd)

	// Here you will define your flags and configuration settings.

	// Cobra supports Persistent Flags which will work for this command
	// and all subcommands, e.g.:
	// serveCmd.PersistentFlags().String("foo", "", "A help for foo")

	// Cobra supports local flags which will only run when this command
	// is called directly, e.g.:
	// serveCmd.Flags().BoolP("toggle", "t", false, "Help message for toggle")
}
