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

package monitoring

import (
	"net/http"

	"github.com/prometheus/client_golang/prometheus/promhttp"
	"go.uber.org/zap"
)

type Exporter struct {
	httpServer *http.Server
	log *zap.SugaredLogger
}

func NewExporter(address string, l *zap.SugaredLogger) (*Exporter, error) {
	e := &Exporter{
		log: l.Named("Exporter"),
	}

	handlers := http.NewServeMux()

	e.httpServer = &http.Server{
		Addr: address,
		Handler: handlers,
	}
	handlers.Handle("/metrics", promhttp.Handler())

	go func(e *Exporter) {
		if err := e.httpServer.ListenAndServe(); err != nil {
			if err == http.ErrServerClosed {
				e.log.Info("Server closing")
			} else {
				e.log.Fatalw("Server failed", "error", err)
			}
		}
	}(e)

	e.log.Debugw("Prometheus exporter started", "address", address)

	return e, nil
}

func (e *Exporter) Close() error {
	if e.httpServer != nil {
		return e.httpServer.Close()
	}
	return nil
}
