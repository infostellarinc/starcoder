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
	"github.com/infostellarinc/starcoder/server"
	"net"
	"log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"
)

const (
	defaultBindAddress = ":50051"
)

// Local flags used to configure the serve command
type serveCmdConfiguration struct {
	BindAddress string
	FlowgraphDir string
}

var serveCmdConfig = serveCmdConfiguration{}

// serveCmd represents the serve command
var serveCmd = &cobra.Command{
	Use:   "serve",
	Short: "Start the Starcoder server",
	Long: `Start the Starcoder server`,
	Run: func(cmd *cobra.Command, args []string) {
		log.Printf("serve called, using bind address %v", serveCmdConfig.BindAddress)

		if serveCmdConfig.FlowgraphDir == "" {
			serveCmdConfig.FlowgraphDir = ""
		}

		lis, err := net.Listen("tcp", serveCmdConfig.BindAddress)
		if err != nil {
			log.Fatalf("failed to listen: %v", err)
		}
		s := grpc.NewServer()
		pb.RegisterProcessManagerServer(s, &server.Starcoder{
			FlowgraphDir: serveCmdConfig.FlowgraphDir,
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
	serveCmd.Flags().StringVar(&serveCmdConfig.BindAddress, "bindAddress", defaultBindAddress, "Address to bind to")
	serveCmd.Flags().StringVar(&serveCmdConfig.FlowgraphDir, "flowgraphDir", "", "Directory containing GNURadio flowgraphs to serve. If blank, defaults to built-in Starcoder flowgraphs.")
}
