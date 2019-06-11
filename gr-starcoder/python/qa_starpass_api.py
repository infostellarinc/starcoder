from gnuradio import gr, gr_unittest
from gnuradio import blocks

from starpass_api import starpass_api

from stellarstation.api.v1.groundstation import groundstation_pb2
from stellarstation.api.v1 import transport_pb2
import grpc_testing
import grpc

import time
import numpy as np
import pmt


class qa_starpass_api (gr_unittest.TestCase):
    def setUp (self):
        service_descriptors = [
            groundstation_pb2.DESCRIPTOR.services_by_name['GroundStationService']
        ]
        self.test_channel = grpc_testing.channel(service_descriptors, grpc_testing.strict_real_time())

        self.tb = gr.top_block ()

    def tearDown (self):
        self.test_channel.close()
        self.test_channel = None
        self.tb = None

    def test_001_correct_initial_request(self):
        gs_id = "1"
        stream_tag = "2"

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        starpass_blk = starpass_api("", "", "", gs_id, "1", stream_tag, [], True, test_channel=self.test_channel)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (starpass_blk, 'in'))
        self.tb.msg_connect((starpass_blk, 'command'), (snk, 'store'))

        self.tb.start()
        invocation_metadata, test_stream = self.test_channel.take_stream_stream(
            groundstation_pb2.DESCRIPTOR
                .services_by_name['GroundStationService']
                .methods_by_name['OpenGroundStationStream'])
        initial_request = test_stream.take_request()
        self.tb.stop()
        test_stream.requests_closed()
        test_stream.terminate({}, grpc.StatusCode.OK, '')
        self.tb.wait()
        self.assertEqual(initial_request, groundstation_pb2.GroundStationStreamRequest(ground_station_id=gs_id,
                                                                           stream_tag=stream_tag))

    def test_002_correctly_passes_on_commands_from_star_receiver(self):
        gs_id = "1"
        plan_id = "3"
        stream_tag = "2"

        get_expected_pmt = lambda x: pmt.cons(pmt.PMT_NIL,
                                              pmt.to_pmt(np.fromstring(x, dtype=np.uint8)))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        starpass_blk = starpass_api("", "", "", gs_id, plan_id, stream_tag, [], True, test_channel=self.test_channel)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (starpass_blk, 'in'))
        self.tb.msg_connect((starpass_blk, 'command'), (snk, 'store'))

        self.tb.start()

        invocation_metadata, test_stream = self.test_channel.take_stream_stream(
            groundstation_pb2.DESCRIPTOR
                .services_by_name['GroundStationService']
                .methods_by_name['OpenGroundStationStream'])

        test_stream.send_response(groundstation_pb2.GroundStationStreamResponse(
            plan_id=plan_id,
            satellite_commands=groundstation_pb2.SatelliteCommands(
                command=['command1', 'command2', 'command3']
            )
        ))
        test_stream.send_response(groundstation_pb2.GroundStationStreamResponse(
            plan_id=plan_id,
            satellite_commands=groundstation_pb2.SatelliteCommands(
                command=['command4', 'command5', 'command6']
            )
        ))
        time.sleep(0.1)

        self.tb.stop()
        test_stream.requests_closed()
        test_stream.terminate({}, grpc.StatusCode.OK, '')
        self.tb.wait()
        self.assertEqual(snk.num_messages(), 6)
        self.assertTrue(pmt.equal(snk.get_message(0), get_expected_pmt('command1')))
        self.assertTrue(pmt.equal(snk.get_message(1), get_expected_pmt('command2')))
        self.assertTrue(pmt.equal(snk.get_message(2), get_expected_pmt('command3')))
        self.assertTrue(pmt.equal(snk.get_message(3), get_expected_pmt('command4')))
        self.assertTrue(pmt.equal(snk.get_message(4), get_expected_pmt('command5')))
        self.assertTrue(pmt.equal(snk.get_message(5), get_expected_pmt('command6')))

    def test_003_correctly_passes_telemetry_to_star_receiver(self):
        gs_id = "1"
        plan_id = "3"
        stream_tag = "2"

        t1 = transport_pb2.Telemetry(
            framing=transport_pb2.WATERFALL,
            data='telemetry',
        )
        in_pmt1 = pmt.intern(t1.SerializeToString())
        t2 = transport_pb2.Telemetry(
            framing=transport_pb2.WATERFALL,
            data='telemetry2',
        )
        in_pmt2 = pmt.intern(t2.SerializeToString())

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        starpass_blk = starpass_api("", "", "", gs_id, plan_id, stream_tag, [], True, test_channel=self.test_channel)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (starpass_blk, 'in'))
        self.tb.msg_connect((starpass_blk, 'command'), (snk, 'store'))

        self.tb.start()

        invocation_metadata, test_stream = self.test_channel.take_stream_stream(
            groundstation_pb2.DESCRIPTOR
                .services_by_name['GroundStationService']
                .methods_by_name['OpenGroundStationStream'])

        starpass_blk.to_basic_block()._post(pmt.intern('in'), in_pmt1)
        starpass_blk.to_basic_block()._post(pmt.intern('in'), in_pmt2)
        initial_request = test_stream.take_request()
        telemetry_request1 = test_stream.take_request()
        telemetry_request2 = test_stream.take_request()

        self.tb.stop()
        test_stream.requests_closed()
        test_stream.terminate({}, grpc.StatusCode.OK, '')
        self.tb.wait()
        self.assertEqual(telemetry_request1, groundstation_pb2.GroundStationStreamRequest(
            ground_station_id=gs_id,
            stream_tag=stream_tag,
            satellite_telemetry=groundstation_pb2.SatelliteTelemetry(
                plan_id=plan_id,
                telemetry=t1,
            )))
        self.assertEqual(telemetry_request2, groundstation_pb2.GroundStationStreamRequest(
            ground_station_id=gs_id,
            stream_tag=stream_tag,
            satellite_telemetry=groundstation_pb2.SatelliteTelemetry(
                plan_id=plan_id,
                telemetry=t2,
            )))


if __name__ == '__main__':
    gr_unittest.run(qa_starpass_api, "qa_starpass_api.xml")
