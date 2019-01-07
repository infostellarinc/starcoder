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
import satellites  # gr-satellites
from pmt_pass import pmt_pass


class rs_encoder(gr.hier_block2):
    """
    This hierarchical block simply wraps around the Encode RS block in
    gr-satellites. It adds an additional parameter `enabled` to determine
    whether to decode the packet or let it through without change.
    """
    def __init__(self, basis, enabled):
        gr.hier_block2.__init__(self,
            "rs_encoder",
            gr.io_signature(0, 0, 0),  # Input signature
            gr.io_signature(0, 0, 0))  # Output signature

        self.log = gr.logger("log")

        self.message_port_register_hier_in("in")
        self.message_port_register_hier_out("out")

        if enabled:
            dec_block = satellites.encode_rs(basis)
        else:
            dec_block = pmt_pass()

        self.msg_connect((self, 'in'), (dec_block, 'in'))
        self.msg_connect((dec_block, 'out'), (self, 'out'))
