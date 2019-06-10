from gnuradio import gr, gr_unittest
from gnuradio import blocks

from starpass_api import starpass_api

from stellarstation.api.v1.groundstation import groundstation_pb2
import grpc_testing

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
        #self.test_channel.close()
        self.test_channel = None
        self.tb = None

    def test_001_default(self):
        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        starpass_blk = starpass_api("", "", "", "1", "1", "1", [], True, test_channel=self.test_channel)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (starpass_blk, 'in'))
        self.tb.msg_connect((starpass_blk, 'command'), (snk, 'store'))

        self.tb.start()

        raise('started tb')

        invocation_metadata, test_stream = self.test_channel.take_stream_stream(
            groundstation_pb2.DESCRIPTOR
                .services_by_name['GroundStationService']
                .methods_by_name['OpenGroundStationStream'])

        print('finished taking stream')

        print(invocation_metadata)
        print(test_stream)

        self.tb.stop()
        test_stream.cancelled()
        self.tb.wait()

        # check data
"""
    def test_001_no_change(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(orig_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(0, 0)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))
"""

if __name__ == '__main__':
    gr_unittest.run(qa_starpass_api, "qa_starpass_api.xml")
