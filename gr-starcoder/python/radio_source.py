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
from gnuradio import uhd
from grc_gnuradio import blks2 as grc_blks2
from starcoder import starcoder_swig

class radio_source(gr.hier_block2):
    def __init__(self, radio, device_address, samp_rate, center_freq, gain, antenna, port_number):
        """
        :param radio: Which radio to use: Currently supports "USRP" or "AR2300" or "TCP"
        :type radio: string
        :param device_address: Device address for USRP or TCP.
        :type device_address: string
        :param samp_rate: the sampling rate
        :type samp_rate: int
        :param center_freq: the center frequency
        :type center_freq: double
        :param gain: Receiver gain
        :type gain: int
        :param antenna: The antenna to use
        :type antenna: string
        :param port_number: Port number for TCP
        :type port_number: int
        """
        gr.hier_block2.__init__(self,
            "radio_source",
            gr.io_signature(0, 0, 0),  # Input signature
            gr.io_signature(1, 1, gr.sizeof_gr_complex)) # Output signature

        # Define blocks and connect them
        if radio == "AR2300":
            self.radio_block = starcoder_swig.ar2300_source()
        elif radio == "USRP":
            self.radio_block = uhd.usrp_source(
                device_address,
                uhd.stream_args(
                    cpu_format="fc32",
                    channels=range(1),
                )
            )
            self.radio_block.set_samp_rate(samp_rate)
            self.radio_block.set_center_freq(center_freq, 0)
            self.radio_block.set_gain(gain, 0)
            self.radio_block.set_antenna(antenna, 0)
        elif radio == "TCP":
            self.radio_block = grc_blks2.tcp_source(
                itemsize=gr.sizeof_gr_complex*1,
                addr=device_address,
                port=port_number,
                server=False,  # TODO: We usually use as client although might be better to make this configurable.
            )
        else:
            raise NotImplementedError
        self.connect(self.radio_block, self)
