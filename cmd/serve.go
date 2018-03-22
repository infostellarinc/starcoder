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
	"github.com/GeertJohan/go.rice"
	pb "github.com/infostellarinc/starcoder/api"
	"github.com/infostellarinc/starcoder/server"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"
	"io/ioutil"
	"log"
	"net"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"syscall"
)

const (
	defaultBindAddress = ":50051"
)

// Local flags used to configure the serve command
type serveCmdConfiguration struct {
	BindAddress  string
	FlowgraphDir string
}

var serveCmdConfig = serveCmdConfiguration{}

// serveCmd represents the serve command
var serveCmd = &cobra.Command{
	Use:   "serve",
	Short: "Start the Starcoder server",
	Long:  `Start the Starcoder server`,
	Run: func(cmd *cobra.Command, args []string) {
		log.Printf("serve called, using bind address %v", serveCmdConfig.BindAddress)

		if serveCmdConfig.FlowgraphDir == "" {
			tempDir, err := ioutil.TempDir("", "starcoder")
			defer os.RemoveAll(tempDir)
			if err != nil {
				log.Fatalf("failed to create directory: %v", err)
			}
			flowgraphsBox := rice.MustFindBox("../flowgraphs")
			flowgraphsBox.Walk("", func(p string, info os.FileInfo, err error) error {
				if p == "" {
					return nil
				}
				if strings.HasSuffix(p, ".grc") || strings.HasSuffix(p, ".py") {
					// Write flowgraph to temporary directory
					err := ioutil.WriteFile(filepath.Join(tempDir, p), flowgraphsBox.MustBytes(p), 0644)
					if err != nil {
						log.Fatalf("failed to write file: %v", err)
					}
				}
				return nil
			})

			serveCmdConfig.FlowgraphDir = tempDir
			log.Printf("Using temporary directory %v", serveCmdConfig.FlowgraphDir)
		}

		lis, err := net.Listen("tcp", serveCmdConfig.BindAddress)
		if err != nil {
			log.Fatalf("failed to listen: %v", err)
		}
		s := grpc.NewServer()
		starcoder := server.NewStarcoderServer(serveCmdConfig.FlowgraphDir)

		// Handle OS signals
		sigs := make(chan os.Signal, 1)
		signal.Notify(sigs)
		go func(s *grpc.Server) {
			for {
				select {
				case sig := <-sigs:
					switch sig {
					case syscall.SIGINT:
						fallthrough
					case syscall.SIGTERM:
						fallthrough
					case syscall.SIGQUIT:
						log.Println("Caught signal", sig)
						starcoder.Close()
						s.GracefulStop()
						return
					}
				}
			}
		}(s)

		pb.RegisterStarcoderServer(s, starcoder)

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
	serveCmd.Flags().StringVar(&serveCmdConfig.BindAddress, "bind-address", defaultBindAddress, "Address to bind to")
	serveCmd.Flags().StringVar(&serveCmdConfig.FlowgraphDir, "flowgraph-dir", "", "Directory containing GNURadio flowgraphs to serve. If blank, defaults to built-in Starcoder flowgraphs.")

	viper.BindPFlags(serveCmd.Flags())
}
