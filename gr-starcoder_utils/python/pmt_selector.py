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
import pmt


class pmt_selector(gr.sync_block):
    """
    This block routes PMTs received into one of two paths,
    selected by `output_index`.
    """
    def __init__(self, output_index):
        gr.sync_block.__init__(self,
            name="pmt_selector",
            in_sig=None,
            out_sig=None)
        self.message_port_register_in(pmt.intern("in"))
        self.message_port_register_out(pmt.intern("out0"))
        self.message_port_register_out(pmt.intern("out1"))
        self.set_msg_handler(pmt.intern("in"), self.msg_handler)
        self.output_index = output_index

    def msg_handler(self, msg):
        # TODO: Have an arbitrary number of outputs
        if self.output_index == 0:
            self.message_port_pub(pmt.intern("out0"), msg)
        if self.output_index == 1:
            self.message_port_pub(pmt.intern("out1"), msg)

    def work(self, input_items, output_items):
        return len(output_items[0])
