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

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import numpy as np
import pmt
import starcoder_utils_swig as starcoder_utils
import tempfile
import os
import time


class qa_folder_source_pdu (gr_unittest.TestCase):
    def setUp (self):
        self.tb = gr.top_block ()
        self.tempdir_path = tempfile.mkdtemp()

        self.tempfiles = []
        self.bytes1 = [os.urandom(259) for _ in range(10)]
        self.bytes2 = [os.urandom(260) for _ in range(10)]
        all_bytes = self.bytes1 + self.bytes2
        for i in range(20):
            handle, filename = tempfile.mkstemp(dir=self.tempdir_path)

            with open(filename, 'w') as f:
                f.write(all_bytes[i])
            os.close(handle)

            self.tempfiles.append(filename)

    def tearDown (self):
        self.tb = None
        for filename in self.tempfiles:
            try:
                os.remove(filename)
            except OSError:
                pass
        try:
            os.rmdir(self.tempdir_path)
        except OSError:
            pass

    def test_001_regular(self):
        src = starcoder_utils.folder_source_pdu(self.tempdir_path, 259, 0)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'out'), (snk, 'store'))

        self.tb.start()
        time.sleep(1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 10)
        bytesRetrieved = {pmt.to_python(snk.get_message(i))[1].tobytes() for i in range(10)}
        for i in range(10):
            self.assertTrue(
                self.bytes1[i] in bytesRetrieved
            )

    def test_002_regular(self):
        src = starcoder_utils.folder_source_pdu(self.tempdir_path, 260, 0)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'out'), (snk, 'store'))

        self.tb.start()
        time.sleep(1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.num_messages(), 10)
        bytesRetrieved = {pmt.to_python(snk.get_message(i))[1].tobytes() for i in range(10)}
        for i in range(10):
            self.assertTrue(
                self.bytes2[i] in bytesRetrieved
            )


if __name__ == '__main__':
    gr_unittest.run(qa_folder_source_pdu, "qa_folder_source_pdu.xml")
