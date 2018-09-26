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

from gnuradio import gr
import pmt

class pdu_trim_uvector(gr.sync_block):
    """
    This block takes in PDUs and trims out the first start_trim_length bytes
    and last end_trim_length bytes of the contained uniform vector.
    This is useful when working with packets where you want to trim out
    headers or ending hashes.
    """
    def __init__(self, start_trim_length, end_trim_length):
        gr.sync_block.__init__(self,
                               name="pdu_trim_uvector",
                               in_sig=None,
                               out_sig=None)
        self.start_trim_length = start_trim_length
        self.end_trim_length = end_trim_length
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
        pmt.set_cdr(msg, pmt.to_pmt(arr[self.start_trim_length:-self.end_trim_length or len(arr)]))
        self.message_port_pub(pmt.intern("out"), msg)

    def work(self, input_items, output_items):
        return len(output_items[0])
