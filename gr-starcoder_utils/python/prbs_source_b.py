#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 Free Software Foundation, Inc..
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

import numpy
from gnuradio import gr
import prbs_generator

class prbs_source_b(gr.sync_block):
    """
    PRBS Source returns pseudorandom bits (unpacked) out as a stream.
    """
    def __init__(self, packet_len=223*8, reset_len=100000, num_packets=0):
        """
        :param packet_len: Length in bits of packet
        :type packet_len: int
        :param reset_len: Number of bits before resetting the PRBS
        :type reset_len: int
        :param num_packets: Number of packets to send out before stopping. If 0,
        this block will keep sending out bits.
        :type num_packets: int
        """
        gr.sync_block.__init__(self,
            name="prbs_source_b",
            in_sig=None,
            out_sig=[numpy.uint8])
        self.base = prbs_generator.PRBSGenerator(reset_len=reset_len)
        self.packet_len = packet_len
        self.num_packets = num_packets

        self.packets_sent = 0

        self.set_output_multiple(packet_len)

    def work(self, input_items, output_items):
        out = output_items[0]
        nout = len(out)
        if self.num_packets == 0:
            gen = self.base.generate_n_bits(nout)
            out[:] = gen[:]
            return nout
        else:
            if self.packets_sent == self.num_packets:
                print('Done sending all bits')
                return -1  # Signals to the flowgraph that we have no more data to provide
            packets_requested = nout / self.packet_len
            packets_left_to_send = self.num_packets - self.packets_sent
            packets_to_send_now = min(packets_requested, packets_left_to_send)
            bits_to_send_now = packets_to_send_now*self.packet_len
            out[:bits_to_send_now] = self.base.generate_n_bits(bits_to_send_now)
            self.packets_sent += packets_to_send_now
            return bits_to_send_now
