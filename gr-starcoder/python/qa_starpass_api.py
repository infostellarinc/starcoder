from gnuradio import gr, gr_unittest
from gnuradio import blocks

from starpass_api import starpass_api

from stellarstation.api.v1.groundstation import groundstation_pb2
import grpc_testing

import time
import threading
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
        #self.test_channel.close()
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
        req = test_stream.take_request()
        test_stream.terminate({}, 0, 'a')
        self.tb.stop()
        self.tb.wait()
        self.assertEqual(req, groundstation_pb2.GroundStationStreamRequest(ground_station_id=gs_id,
                                                                           stream_tag=stream_tag))
        print(req)
        return


if __name__ == '__main__':
    gr_unittest.run(qa_starpass_api, "qa_starpass_api.xml")
