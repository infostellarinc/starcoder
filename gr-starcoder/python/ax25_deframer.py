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

import numpy
import pmt
from gnuradio import gr

class ax25_deframer(gr.sync_block):
    """
    Removes the frame header from an AX.25 frame and returns it as metadata.
    """
    def __init__(self):
        gr.sync_block.__init__(self,
            name="AX25 Deframer",
            in_sig=None,
            out_sig=None)
        self.message_port_register_in(pmt.intern("in"))
        self.message_port_register_out(pmt.intern("out"))
        self.set_msg_handler(pmt.intern("in"), self.msg_handler)

    def msg_handler(self, msg):
        if not pmt.is_blob(msg):
            return

        # Add code here to process pmt.blob_data

        self.message_port_pub(pmt.intern("out"), msg)
