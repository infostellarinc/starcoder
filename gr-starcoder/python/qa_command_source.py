#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 Infostellar, Inc.
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
import starcoder_swig as starcoder
import time
import pmt
import starcoder_pb2


class qa_command_source (gr_unittest.TestCase):
    # TODO: Complete tests for each PMT type
    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_complex(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        msg.complex_value.real_value = 1.2
        msg.complex_value.imaginary_value = 2.3

        expected = pmt.from_complex(1.2 + 2.3j)

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_complex(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_pair(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        msg.pair_value.car.symbol_value = "testtransmission"
        msg.pair_value.cdr.double_value = 23.2

        expected = pmt.cons(pmt.intern("testtransmission"),
                            pmt.from_double(23.2))

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_pair(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_tuple(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        v = msg.list_value.value.add()
        v.symbol_value = "testtransmission"
        v = msg.list_value.value.add()
        v.double_value = 23.2
        msg.list_value.type = starcoder_pb2.List.TUPLE

        expected = pmt.make_tuple(pmt.intern("testtransmission"),
                                  pmt.from_double(23.2))

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_tuple(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_blob(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        msg.blob_value = "data"

        expected = pmt.init_u8vector(4, [ord('d'), ord('a'), ord('t'), ord('a')])

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_u8vector(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_u8_vector(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        msg.uniform_vector_value.u_value.value.extend([12, 0, 3])
        msg.uniform_vector_value.u_value.size = starcoder_pb2.Size8

        expected = pmt.init_u8vector(3, [12, 0, 3])

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_u8vector(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_i32_vector(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        msg.uniform_vector_value.i_value.value.extend([12, -65500, 3])
        msg.uniform_vector_value.i_value.size = starcoder_pb2.Size32

        expected = pmt.init_s32vector(3, [12, -65500, 3])

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_s32vector(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_f64_vector(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        msg.uniform_vector_value.f64_value.value.extend([2.4, -12.3, 21.2])

        expected = pmt.init_f64vector(3, [2.4, -12.3, 21.2])

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_f64vector(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_c64_vector(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        pl = msg.uniform_vector_value.c64_value.value.add()
        pl.real_value = 1.2
        pl.imaginary_value = -2.3
        pl = msg.uniform_vector_value.c64_value.value.add()
        pl.real_value = 3.2
        pl.imaginary_value = -1
        pl = msg.uniform_vector_value.c64_value.value.add()
        pl.real_value = 0
        pl.imaginary_value = -1

        expected = pmt.init_c64vector(3, [1.2-2.3j, 3.2-1j, -1j])

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_c64vector(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_dict(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        pl = msg.dict_value.entry.add()
        pl.key.symbol_value = 'key'
        pl.value.double_value = -2.3

        expected = pmt.make_dict()
        expected = pmt.dict_add(expected, pmt.intern('key'),
                                pmt.from_double(-2.3))

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_dict(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))

    def test_pdu(self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'store'))

        msg = starcoder_pb2.BlockMessage()
        md = msg.pair_value.car.dict_value.entry.add()
        md.key.symbol_value = 'metadata_1'
        md.value.integer_value = -2
        md = msg.pair_value.car.dict_value.entry.add()
        md.key.symbol_value = 'metadata_2'
        md.value.symbol_value = 'val'

        msg.pair_value.cdr.uniform_vector_value.f64_value.value.extend([2.4, -12.3, 21.2])

        pmt_dict = pmt.make_dict()
        pmt_dict = pmt.dict_add(pmt_dict, pmt.intern('metadata_1'), pmt.from_long(-2))
        pmt_dict = pmt.dict_add(pmt_dict, pmt.intern('metadata_2'), pmt.intern('val'))
        expected = pmt.cons(
            pmt_dict,
            pmt.init_f64vector(3, [2.4, -12.3, 21.2])
        )

        self.tb.start()
        cs.push(msg.SerializeToString())
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_dict(snk.get_message(0)))
        self.assertTrue(pmt.equal(snk.get_message(0), expected))


if __name__ == '__main__':
    gr_unittest.run(qa_command_source, "qa_command_source.xml")
