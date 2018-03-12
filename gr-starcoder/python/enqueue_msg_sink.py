#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 <+YOU OR YOUR COMPANY+>.
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
from collections import deque

class enqueue_msg_sink(gr.basic_block):
    """
    docstring for block enqueue_msg_sink
    """
    def __init__(self):
        gr.basic_block.__init__(self,
            name="enqueue_msg_sink",
            in_sig=[],
            out_sig=[])
        self.q = deque()
        self.message_port_register_in(pmt.intern("in"))
        self.set_msg_handler(pmt.intern("in"), self.msg_handler)

    def get_q(self):
        return self.q

    def msg_handler(self, message):
        self.q.append(bytearray(pmt.serialize_str(message)))
