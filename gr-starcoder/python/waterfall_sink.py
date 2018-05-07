#! /usr/bin/python
#
# Copyright 2018 Infostellar. Inc,
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
#  Copyright (C) 2017, Libre Space Foundation <http://librespacefoundation.org/>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#


from gnuradio import gr
from gnuradio import blocks
from starcoder import starcoder_swig


class waterfall_sink(gr.hier_block2):

    """
    Hierarchical block responsible for the creation of the spectrum waterfall
    of the observation
    """

    def __init__(self, samp_rate, center_freq, rps, fft_size, filename, mode):
        """

        :param samp_rate: the sampling rate
        :type samp_rate: double
        :param center_freq: the center frequency
        :type center_freq: double
        :param rps: rows per second
        :type rps: int
        :param fft_size: the FFT size
        :type fft_size: int
        :param filename: the name of the resulting png image
        :type filename: string
        :param mode: the operation mode of the waterfall (0 = simple decimation,
        1 = max hold, 2 = mean)
        :type mode: int
        """
        gr.hier_block2.__init__(self,
                                "waterfall_sink",
                                gr.io_signature(1, 1, gr.sizeof_gr_complex),  # Input signature
                                gr.io_signature(0, 0, 0)) # Output signature

        waterfall_heatmap = starcoder_swig.waterfall_heatmap(samp_rate, center_freq, rps, fft_size, mode)
        s2v = blocks.stream_to_vector(gr.sizeof_gr_complex, fft_size)
        self.waterfall_pl = starcoder_swig.waterfall_plotter(samp_rate, center_freq, rps, fft_size, filename)

        self.connect((self, 0), (s2v, 0))
        self.connect((s2v, 0), (waterfall_heatmap, 0))
        self.connect((waterfall_heatmap, 0), (self.waterfall_pl, 0))

    def register_starcoder_queue(self, ptr):
        self.waterfall_pl.register_starcoder_queue(ptr)
