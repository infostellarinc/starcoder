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
import pmt
from gnuradio import gr
from utils import is_u8_pdu

class add_incrementing_header_pdu(gr.sync_block):
    """
    Adds a 4-byte uint32 header at the beginning of a PDU
    representing the number of valid PDUs that have gone through
    this block. Begins at 0. Little-endian.
    This block works on packed bytes.
    """
    def __init__(self):
        gr.sync_block.__init__(self,
            name="add_incrementing_header_pdu",
            in_sig=None,
            out_sig=None)

        self.message_port_register_in(pmt.intern("in"))
        self.message_port_register_out(pmt.intern("out"))

        self.incrementing_header = 0

        self.set_msg_handler(pmt.intern("in"), self.msg_handler)

    def msg_handler(self, msg):
        if not is_u8_pdu(msg):
            return

        arr = pmt.to_python(msg)[1]
        header_arr = np.frombuffer(np.array([self.incrementing_header], dtype='<u4').tobytes(),
                                   dtype=np.uint8)
        pmt.set_cdr(msg, pmt.to_pmt(
            np.concatenate((header_arr, arr))))
        self.message_port_pub(pmt.intern("out"), msg)
        self.incrementing_header += 1

    def work(self, input_items, output_items):
        return len(output_items[0])
