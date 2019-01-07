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

from gnuradio import gr
from gnuradio import blocks
from gnuradio import digital
from pmt_pass import pmt_pass


class additive_scrambler(gr.hier_block2):
    """
    This hierarchical block simply wraps around the Additive Scrambler block in core
    GNU Radio. It adds an additional parameter `enabled` to determine whether to
    decode the packet or let it through without change.
    """
    def __init__(self, mask, seed, length, count, bits_per_byte, reset_tag_key, enabled):
        gr.hier_block2.__init__(self,
            "additive_scrambler",
            gr.io_signature(0, 0, 0),  # Input signature
            gr.io_signature(0, 0, 0)) # Output signature

        self.message_port_register_hier_in("in")
        self.message_port_register_hier_out("out")

        if enabled:
            pdu_to_tagged_stream = blocks.pdu_to_tagged_stream(blocks.byte_t, 'packet_len')
            digital_additive_scrambler = digital.additive_scrambler_bb(mask, seed, length, count, bits_per_byte, reset_tag_key)
            tagged_stream_to_pdu = blocks.tagged_stream_to_pdu(blocks.byte_t, 'packet_len')

            self.msg_connect((self, 'in'), (pdu_to_tagged_stream, 'pdus'))
            self.connect((pdu_to_tagged_stream, 0), (digital_additive_scrambler, 0))
            self.connect((digital_additive_scrambler, 0), (tagged_stream_to_pdu, 0))
            self.msg_connect((tagged_stream_to_pdu, 'pdus'), (self, 'out'))
        else:
            dec_block = pmt_pass()
            self.msg_connect((self, 'in'), (dec_block, 'in'))
            self.msg_connect((dec_block, 'out'), (self, 'out'))
