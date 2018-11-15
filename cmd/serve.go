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
	"github.com/grpc-ecosystem/go-grpc-middleware"
	"github.com/grpc-ecosystem/go-grpc-middleware/logging/zap"
	"github.com/grpc-ecosystem/go-grpc-middleware/tags"
	pb "github.com/infostellarinc/starcoder/api"
	"github.com/infostellarinc/starcoder/monitoring"
	"github.com/infostellarinc/starcoder/server"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
	"go.uber.org/zap"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"
	"io/ioutil"
	fatalLog "log"
	"net"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"syscall"
	"time"
)

const (
	defaultBindAddress               = ":50051"
	defaultExporterAddress           = ":9999"
	defaultPerfCtrCollectionInterval = time.Second * 15
)

// Local flags used to configure the serve command
type serveCmdConfiguration struct {
	BindAddress               string
	FlowgraphDir              string
	ExporterAddress           string
	PerfCtrCollectionInterval time.Duration
	SilencedCommandBlocks     []string
}

var serveCmdConfig = serveCmdConfiguration{}

// serveCmd represents the serve command
var serveCmd = &cobra.Command{
	Use:   "serve",
	Short: "Start the Starcoder server",
	Long:  `Start the Starcoder server`,
	Run: func(cmd *cobra.Command, args []string) {
		l, err := zap.NewDevelopment()
		if err != nil {
			fatalLog.Fatalf("Failed to create logger: %v", err)
		}
		log := l.Sugar()
		defer log.Sync()

		log.Infof("Using configuration: %+v", serveCmdConfig)
		log.Infof("serve called, using bind address %v", serveCmdConfig.BindAddress)

		// Set up metrics endpoint
		metrics := monitoring.NewMetrics()
		exporter, err := monitoring.NewExporter(serveCmdConfig.ExporterAddress, log)
		if err != nil {
			log.Errorw("Couldn't start Prometheus metrics exporter", "error", err)
		}
		defer func() {
			if exporter != nil {
				exporter.Close()
			}
		}()
		log.Infow("Started Prometheus metrics exporter",
			"address", serveCmdConfig.ExporterAddress)

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
					newPath := filepath.Join(tempDir, p)
					newDirectory := filepath.Dir(newPath)
					err := os.MkdirAll(newDirectory, os.ModePerm)
					if err != nil {
						log.Fatalf("failed to create directory: %v", err)
					}
					// Write flowgraph to temporary directory
					err = ioutil.WriteFile(newPath, flowgraphsBox.MustBytes(p), 0644)
					if err != nil {
						log.Fatalf("failed to write file: %v", err)
					}
				}
				return nil
			})

			serveCmdConfig.FlowgraphDir = tempDir
			log.Infof("Using temporary directory %v", serveCmdConfig.FlowgraphDir)
		}

		lis, err := net.Listen("tcp", serveCmdConfig.BindAddress)
		if err != nil {
			log.Fatalf("failed to listen: %v", err)
		}
		s := grpc.NewServer(
			grpc_middleware.WithStreamServerChain(
				grpc_ctxtags.StreamServerInterceptor(grpc_ctxtags.WithFieldExtractor(grpc_ctxtags.CodeGenRequestFieldExtractor)),
				grpc_zap.StreamServerInterceptor(l),
			),
		)
		starcoder := server.NewStarcoderServer(serveCmdConfig.FlowgraphDir, serveCmdConfig.PerfCtrCollectionInterval, serveCmdConfig.SilencedCommandBlocks, log, metrics)

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
						log.Infof("Caught signal: %v", sig)
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
	serveCmd.Flags().StringVar(&serveCmdConfig.ExporterAddress, "exporter-address", defaultExporterAddress, "Address where exported Prometheus metrics will be served")
	serveCmd.Flags().DurationVar(&serveCmdConfig.PerfCtrCollectionInterval, "perf-ctr-interval", defaultPerfCtrCollectionInterval, "Time interval for exporting GNURadio performance metrics to Prometheus. If set to 0, this will be disabled. Default 15s.")
	serveCmd.Flags().StringSliceVar(&serveCmdConfig.SilencedCommandBlocks, "silenced-command-blocks", []string{}, "Each command sent to a block in Starcoder is logged to the output. This can be too much for blocks that receive commands multiple times a second e.g. Doppler shift blocks. You can provide a list of comma-separated strings to this variable to silence command logging for the block e.g. \"doppler_command_source,doppler_command_source_transmit\"")

	viper.BindPFlags(serveCmd.Flags())
}
