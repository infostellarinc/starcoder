#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2019 Infostellar.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import pmt
from groundstation_api_doppler import groundstation_api_doppler, interpolate_coordinates

import time

from google.protobuf.timestamp_pb2 import Timestamp
from stellarstation.api.v1.groundstation import groundstation_pb2
from stellarstation.api.v1 import common
import grpc
import grpc_testing

class qa_groundstation_api_doppler (gr_unittest.TestCase):

    def setUp (self):
        service_descriptors = [
            groundstation_pb2.DESCRIPTOR.services_by_name['GroundStationService']
        ]
        self.test_channel = grpc_testing.channel(service_descriptors, grpc_testing.strict_real_time())

        self.tb = gr.top_block()

    def tearDown(self):
        self.test_channel.close()
        self.test_channel = None
        self.tb = None

    def test_001_correct_response(self):
        gs_id = "1"
        plan_id = "2"

        groundstation_blk = groundstation_api_doppler(0, 0, "", "", "", gs_id, plan_id, 1, True, test_channel=self.test_channel)
        snk = blocks.message_debug()

        self.tb.msg_connect((groundstation_blk, 'downlink_shift'), (snk, 'store'))

        self.tb.start()
        _, request, unary_unary_channel_rpc = self.test_channel.take_unary_unary(
            groundstation_pb2.DESCRIPTOR
                .services_by_name['GroundStationService']
                .methods_by_name['ListPlans']
        )
        unary_unary_channel_rpc.terminate(groundstation_pb2.ListPlansResponse(
            plan=[groundstation_pb2.Plan(plan_id="1"),
                  groundstation_pb2.Plan(plan_id="2", satellite_coordinates=[
                      groundstation_pb2.SatelliteCoordinates(
                          range_rate=1234.56,
                          time=Timestamp(seconds=int(time.time() + 2))
                      ),
                      groundstation_pb2.SatelliteCoordinates(
                          range_rate=1235.6,
                          time=Timestamp(seconds=int(time.time() + 3))
                      ),
                  ])]
        ), {}, grpc.StatusCode.OK, '')
        self.tb.stop()
        self.tb.wait()
        self.assertEqual(request.ground_station_id, gs_id)

    def test_002_interpolate_coordinates(self):
        now = time.time()
        satellite_coordinates=[
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 2)),
                range_rate = 2.1,
            ),
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 3)),
                range_rate = 2.4,
            ),
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 4)),
                range_rate = 2.9,
            ),
        ]
        interpolated = interpolate_coordinates(satellite_coordinates, 2)
        expected = [
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 2)),
                range_rate = 2.1,
            ),
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 2),
                               nanos=int(0.5 * 10**9)),
                range_rate = 2.25,
            ),
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 3)),
                range_rate = 2.4,
            ),
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 3),
                               nanos=int(0.5 * 10**9)),
                range_rate = 2.65,
            ),
            groundstation_pb2.SatelliteCoordinates(
                time=Timestamp(seconds=int(now + 4)),
                range_rate = 2.9,
            ),
        ]
        self.assertEqual(interpolated, expected)

    def test_003_interpolated_vals_are_sent_out(self):
        gs_id = "1"
        plan_id = "2"

        groundstation_blk = groundstation_api_doppler(1e6, 1e6, "", "", "", gs_id, plan_id, 2, True, test_channel=self.test_channel)
        downlink_sink = blocks.message_debug()
        uplink_sink = blocks.message_debug()

        self.tb.msg_connect((groundstation_blk, 'downlink_shift'), (downlink_sink, 'store'))
        self.tb.msg_connect((groundstation_blk, 'uplink_shift'), (uplink_sink, 'store'))

        now = time.time()

        self.tb.start()
        _, request, unary_unary_channel_rpc = self.test_channel.take_unary_unary(
            groundstation_pb2.DESCRIPTOR
                .services_by_name['GroundStationService']
                .methods_by_name['ListPlans']
        )
        unary_unary_channel_rpc.terminate(groundstation_pb2.ListPlansResponse(
            plan=[groundstation_pb2.Plan(plan_id="1"),
                  groundstation_pb2.Plan(plan_id="2", satellite_coordinates=[
                      groundstation_pb2.SatelliteCoordinates(
                          time=Timestamp(seconds=int(now + 3)),
                          range_rate = 2.1e7,
                      ),
                      groundstation_pb2.SatelliteCoordinates(
                          time=Timestamp(seconds=int(now + 4)),
                          range_rate = 2.4e7,
                      ),
                      groundstation_pb2.SatelliteCoordinates(
                          time=Timestamp(seconds=int(now + 5)),
                          range_rate = 2.9e7,
                      ),
                  ])]
        ), {}, grpc.StatusCode.OK, '')
        time.sleep(6)
        self.tb.stop()
        self.tb.wait()
        self.assertEqual(request.ground_station_id, gs_id)
        self.assertEqual(downlink_sink.num_messages(), 5)
        self.assertEqual(uplink_sink.num_messages(), 5)
        downlink_sink_expected = [1e6-929951.5400083881, 1e6-924948.0785804158, 1e6-919944.6171524435,  1e6-911605.5147724898, 1e6-903266.412392536]
        for i in range(5):
            self.assertAlmostEqual(pmt.to_python(downlink_sink.get_message(i)), downlink_sink_expected[i])
        uplink_sink_expected = [1e6-1070048.459991612, 1e6-1075051.9214195842, 1e6-1080055.3828475566, 1e6-1088394.4852275103, 1e6-1096733.587607464]
        for i in range(5):
            self.assertAlmostEqual(pmt.to_python(uplink_sink.get_message(i)), uplink_sink_expected[i])

if __name__ == '__main__':
    gr_unittest.run(qa_groundstation_api_doppler, "qa_groundstation_api_doppler.xml")
