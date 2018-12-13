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

class add_sync_pdu(gr.sync_block):
    """
    Adds a sync word with byte_length bytes to
    the beginning of the PDU payload. Works on
    unpacked bytes.
    """
    def __init__(self, sync_word, byte_length):
        gr.sync_block.__init__(self,
            name="add_sync_pdu",
            in_sig=None,
            out_sig=None)
        self.sync_word = sync_word
        self.byte_length = byte_length

        self.sync_array = [
            (self.sync_word >> (self.byte_length*8-1-i)) & 1
            for i in range(self.byte_length*8)
        ]
        self.sync_array = np.array(self.sync_array, np.uint8)

        self.message_port_register_in(pmt.intern("in"))
        self.message_port_register_out(pmt.intern("out"))

        self.set_msg_handler(pmt.intern("in"), self.msg_handler)

    def msg_handler(self, msg):
        if not pmt.is_pair(msg):
            return

        if not pmt.is_dict(pmt.car(msg)):
            return

        if not pmt.is_uniform_vector(pmt.cdr(msg)):
            return

        arr = pmt.to_python(msg)[1]
        pmt.set_cdr(msg, pmt.to_pmt(
            np.concatenate((self.sync_array, arr))))
        self.message_port_pub(pmt.intern("out"), msg)

    def work(self, input_items, output_items):
        return len(output_items[0])
