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
	"github.com/prometheus/client_golang/prometheus"
	"fmt"
)

const metricsPrefix = "starcoder_"

var PerformanceCountersToCollect = []string{
	"pc_work_time_total",
	"pc_work_time_avg",
	"pc_work_time",
	"pc_work_time_var",
	"pc_throughput_avg",
	"pc_nproduced",
	"pc_nproduced_avg",
	"pc_nproduced_var",
}

type Metrics struct {
	FlowgraphCount prometheus.Gauge
}

func NewMetrics() *Metrics {
	m := &Metrics{
		FlowgraphCount: prometheus.NewGauge(prometheus.GaugeOpts{
			Name: getMetricsName("flowgraph_count"),
			Help: "Current number of flowgraphs running",
		}),
	}
	m.init()
	return m
}

func (m *Metrics) init() {
	prometheus.MustRegister(m.FlowgraphCount)
}

func getMetricsName(name string) string {
	return metricsPrefix + name
}

// Registers a new gauge. If gauge already exists, returns that.
func GetPerfCtrGauge(flowgraphName string, blockName string, counterName string) (prometheus.Gauge, error) {
	gauge := prometheus.NewGauge(prometheus.GaugeOpts{
		Name: getMetricsName(flowgraphName + "_" + blockName + "_" + counterName),
		Help: fmt.Sprintf(
			"Gauge for performance counter %v of block %v in flowgraph %v",
			counterName, blockName, flowgraphName,
			),
	})
	err := prometheus.Register(gauge)
	if err != nil {
		if val, ok := err.(prometheus.AlreadyRegisteredError); ok {
			gauge = val.ExistingCollector.(prometheus.Gauge)
		} else {
			return nil, err
		}
	}
	return gauge, nil
}
