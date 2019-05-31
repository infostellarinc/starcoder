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

class starpass_api(gr.sync_block):
    """
    docstring for block starpass_api
    """
    def __init__(self, api_key, api_url, root_cert_path, groundstation_id, stream_tag, files_to_collect):
        gr.sync_block.__init__(self,
            name="starpass_api",
            in_sig=None,
            out_sig=None)
        self.api_key = api_key
        self.api_url = api_url
        self.root_cert_path = root_cert_path
        self.groundstation_id = groundstation_id
        self.stream_tag = stream_tag
        self.files_to_collect = files_to_collect
        print(
            self.api_key,
            self.api_url,
            self.root_cert_path,
            self.groundstation_id,
            self.stream_tag,
            self.files_to_collect,
        )
        self.message_port_register_out(pmt.intern("command"))
        self.message_port_register_out(pmt.intern("receive_frequency_shift"))
        self.message_port_register_out(pmt.intern("transmit_frequency_shift"))

        self.message_port_register_in(pmt.intern("in"))
        self.set_msg_handler(pmt.intern("in"), self.msg_handler)

    def msg_handler(self, msg):
        pass

    def work(self, input_items, output_items):
        return len(output_items[0])
