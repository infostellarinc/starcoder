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
from gnuradio import blocks
from gnuradio import analog
from gnuradio import filter
from radio_source import radio_source

class iq_only_receiver(gr.hier_block2):
    """
    This hierarchical block wraps a `starcoder.radio_source` and performs the
    following functions:
    * Expects Doppler shift information from an external source for correction.
    * The offset frequency for preventing DC bias can be specified.
    * Resampling from the original radio sampling rate to the target sampling rate.
    """
    def __init__(self, radio, radio_device_address, radio_samp_rate, radio_center_freq, radio_gain, radio_antenna, target_samp_rate, freq_offset_dc_bias, radio_port_number):
        """
        :param radio: Which radio to use: Currently supports "USRP" or "AR2300" or "TCP"
        :type radio: string
        :param radio_device_address: Device address.
        :type radio_device_address: string
        :param radio_samp_rate: the sampling rate
        :type radio_samp_rate: int
        :param radio_center_freq: the center frequency
        :type radio_center_freq: double
        :param radio_gain: Receiver gain
        :type radio_gain: int
        :param radio_antenna: The antenna to use
        :type radio_antenna: string
        :param target_samp_rate: Target sampling rate
        :type target_samp_rate: int
        :param freq_offset_dc_bias: Frequency offset to eliminate DC bias
        :type freq_offset_dc_bias: double
        :param radio_port_number: Port number for TCP
        :type radio_port_number: int
        """
        gr.hier_block2.__init__(self,
            "iq_only_receiver",
            gr.io_signature(0, 0, 0),  # Input signature
            gr.io_signature(1, 1, gr.sizeof_gr_complex)) # Output signature

        self.log = gr.logger("log")

        self.message_port_register_hier_in("doppler_in")

        # Define blocks and connect them
        if radio == "AR2300" and radio_samp_rate != 1125000:
            self.log.warn("Radio sampling rate not set to 1125000 for AR2300. AR2300 only supports 1125000.")
            radio_samp_rate = 1125000
        if (radio == "AR2300" or radio == "TCP") and freq_offset_dc_bias != 0:
            self.log.warn("It is not possible to offset DC bias since "
                          "frequency setting is done outside GNU Radio.")
            freq_offset_dc_bias = 0
        if radio == "USRP":
            available_bw = min(radio_samp_rate/2 - abs(freq_offset_dc_bias), abs(freq_offset_dc_bias))
            self.log.debug("Available bandwidth for signal after DC bias correction "
                           "and before resampling is {}. "
                           "This includes signal bandwidth + total Doppler shift".format(available_bw))

        if target_samp_rate > radio_samp_rate:
            self.log.warn("Target sampling rate is larger than radio sampling rate.")

        self.radio = radio_source(radio, radio_device_address, radio_samp_rate, radio_center_freq+freq_offset_dc_bias, radio_gain, radio_antenna, radio_port_number)
        self.mult = blocks.multiply_vcc(1)
        self.rational_resampler = filter.rational_resampler_ccc(
            interpolation=target_samp_rate,
            decimation=radio_samp_rate,
            taps=None,
            fractional_bw=None,
        )
        self.dc_bias_correction_source = analog.sig_source_c(radio_samp_rate, analog.GR_COS_WAVE, freq_offset_dc_bias, 1, 0)

        self.connect((self.radio, 0), (self.mult, 0))
        self.connect((self.dc_bias_correction_source, 0), (self.mult, 1))
        if radio != "AR2300":
            # Doppler correction is done in software
            self.doppler_correction_source = analog.sig_source_c(radio_samp_rate, analog.GR_COS_WAVE, 0, 1, 0)
            self.msg_connect((self, 'doppler_in'), (self.doppler_correction_source, 'freq'))
            self.connect((self.doppler_correction_source, 0), (self.mult, 2))
        self.connect((self.mult, 0), (self.rational_resampler, 0))
        self.connect((self.rational_resampler, 0), (self, 0))
