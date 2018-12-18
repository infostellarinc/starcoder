#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 Infostellar.
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

import numpy as np
from gnuradio import gr
import pmt
import prbs_generator


class prbs_sink_pdu(gr.sync_block):
    """
    docstring for block prbs_sink_pdu
    """
    def __init__(self, packet_len_bits_no_header, reset_len, num_packets):
        gr.sync_block.__init__(self,
            name="prbs_sink_pdu",
            in_sig=None,
            out_sig=None)
        self.log = gr.logger("log")

        self.packet_len_bits_no_header = packet_len_bits_no_header
        self.reset_len = reset_len
        self.num_packets = num_packets

        self.generator = prbs_generator.PRBSGenerator(reset_len=reset_len)

        self.all_packets_received_ctr = 0
        self.wrong_length_packets_after_fec_ctr = 0
        self.erroneous_packets_after_fec_ctr = 0
        self.correct_packets_received_ctr = 0
        self.collected_packets = np.zeros(self.num_packets)

        self.message_port_register_in(pmt.intern("all"))
        self.message_port_register_in(pmt.intern("corrected"))

        self.set_msg_handler(pmt.intern("all"), self.all_handler)
        self.set_msg_handler(pmt.intern("corrected"), self.corrected_handler)

    def all_handler(self, msg):
        if not pmt.is_pair(msg):
            return

        if not pmt.is_dict(pmt.car(msg)):
            return

        if not pmt.is_u8vector(pmt.cdr(msg)):
            return

        self.all_packets_received_ctr += 1

    def corrected_handler(self, msg):
        if not pmt.is_pair(msg):
            return

        if not pmt.is_dict(pmt.car(msg)):
            return

        if not pmt.is_u8vector(pmt.cdr(msg)):
            return

        arr = pmt.to_python(msg)[1]
        if len(arr) != self.packet_len_bits_no_header/8 + 4:
            self.log.warn("Received packet with the wrong length. Expecting {} got {}".format(
                self.packet_len_bits_no_header/8 + 4,
                len(arr)))
            self.wrong_length_packets_after_fec_ctr += 1
            return
        packet_idx = self.verify_packet_and_return_idx(arr)
        if packet_idx == -1:
            self.log.warn("Received erroneous packets even after FEC.")
            self.erroneous_packets_after_fec_ctr += 1
            return
        self.correct_packets_received_ctr += 1
        self.collected_packets[packet_idx] += 1

    def verify_packet_and_return_idx(self, arr):
        packet_idx = arr[3] << 24
        packet_idx |= arr[2] << 16
        packet_idx |= arr[1] << 8
        packet_idx |= arr[0]

        number_bits_sent_before_this_packet = self.packet_len_bits_no_header * packet_idx
        expected = self.generator.generate_n_bits_after_x(self.packet_len_bits_no_header,
                                                          number_bits_sent_before_this_packet)
        # Pack the expected bits
        packed_expected = np.zeros(self.packet_len_bits_no_header/8, dtype=np.uint8)
        idx = 0
        packed_expected_idx = 0
        packed_byte = 0
        for bit in expected:
            packed_byte |= bit << (7-idx)
            if idx == 7:
                packed_expected[packed_expected_idx] = packed_byte
                packed_expected_idx += 1
                packed_byte = 0
                idx = 0
            else:
                idx += 1

        if not np.array_equal(packed_expected, arr[4:]):
            return -1

        return packet_idx

    def print_report(self):
        print("Expected number of packets send: {}".format(
            self.num_packets
        ))
        print("Total number of packets received: {}".format(
            self.all_packets_received_ctr
        ))
        print("Total number of correct packets after FEC: {}".format(
            self.correct_packets_received_ctr
        ))
        print("Total number of wrong length packets after FEC (This should be 0): {}".format(
            self.wrong_length_packets_after_fec_ctr
        ))
        print("Total number of erroneous packets after FEC (This should be 0): {}".format(
            self.erroneous_packets_after_fec_ctr
        ))
        print("Total number of unique packets after FEC: {}".format(
            np.count_nonzero(self.collected_packets)
        ))
        print("Total number of duplicates: {}".format(
            np.sum(self.collected_packets[np.where(self.collected_packets > 1)] - 1)
        ))
        print("Frame error rate: {}".format(
            1 - np.count_nonzero(self.collected_packets) / float(self.num_packets)
        ))

    def stop(self):
        self.print_report()
        return True

    def work(self, input_items, output_items):
        return len(input_items[0])
