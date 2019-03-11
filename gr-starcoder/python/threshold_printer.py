#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
#############################
#
# starcoder_CW_threshold_printer
#
# The aim of this "asyncronous" GNURadio python block is to read a tagged stream of values
# containing the Mag^2 of the signal, with RMS, multiplied by 2 (amplitude) and decimated 10.000 times.
# This reading corresponds to the averaged amplitude of the received signal.
# This block processes tries to detect when a morse signal is incoming, and adjusts its output to be
# a suitable threshold value for the "CW to Symbols" block (modified to accept real time threshold vars).
# When no morse signal is detected, it sets a threshold slightly over the noise in order to avoid
# reading erroneous symbols.
#
# By selecting "Straight Mode" to True, the decided threshold will be such that only signal
# with higher than "Hard Threshold" power value is readed. By selecting False, in addition to the
# "Hard Threshold" value, other aspects are taken into account as well.

from gnuradio import gr
import pmt


class threshold_printer(gr.basic_block):
    def __init__(self, straight_mode=False, hard_threshold=20):
        gr.basic_block.__init__(
            self,
            name='Threshold printer',
            in_sig=None,
            out_sig=None,
        )

        self.hard_threshold = hard_threshold
        self.straight_mode = straight_mode
        self.triggered = 0
        self.msga = 1000
        self.message_port_register_out(pmt.intern('out'))
        self.message_port_register_in(pmt.intern('in'))
        self.set_msg_handler(pmt.intern('in'), self.handle_msg)

    def handle_msg(self, msg):
        # Convert PMT to python object (is a tuple). Extract 1st
        # element of tuple (is np.ndarray) and extract 1st element
        # of ndarray. Finally we get a float32.
        msg0 = pmt.to_python(msg)[1][0]

        # Straight mode
        if self.straight_mode:
            if msg0 > self.hard_threshold:  # When the signal exceeds "hard threshold".
                thr0 = float(msg0 / 3)  # Take 1/3 of it's power as threshold.
            else:
                thr0 = float(msg0 + 3)  # Keep the threshold away from the noise.
        # Normal mode
        else:
            # When it is not triggered and
            # a sudden jump is detected.
            if (self.triggered < 3) & (msg0/self.msga > 6):
                thr0 = float(msg0 / 2)  # Take 1/2 of it's power as threshold.
                self.triggered = 15	 # Set the trigger.
            elif self.triggered > 3:
                thr0 = float(msg0 / 2)  # Signal 1/2 when triggered.
            else:
                thr0 = float(msg0 + 3)  # Stay away from noise.

            if msg0 > self.hard_threshold:  # When the signal exceeds "hard threshold".
                thr0 = float(msg0 / 3)  # Take 1/3 of it's power as threshold.

            self.msga = msg0  # Previous value container.

        self.triggered = self.triggered - 1.  # Trigger discounter.

        thr = pmt.to_pmt(thr0)  # Convert the "1/N" threshold value in msg0 to pmt
        thr_pair = pmt.cons(thr, thr)  # Convert thr to a "pmt pair"
        self.message_port_pub(pmt.intern('out'), thr_pair)  # Send the thr to "CW to Symbols".
