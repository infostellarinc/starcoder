/* -*- c++ -*- */
/*
 * Copyright 2018 Infostellar, Inc.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "qa_meteor_decoder.h"
#include <cppunit/TestAssert.h>
#include <stdio.h>

#include "meteor_decoder.h"
#include "meteor_packet.h"

namespace gr {
namespace starcoder {

void qa_meteor_decoder::test_full_decoding() {
  gr::starcoder::meteor_decoder decoder;
  gr::starcoder::meteor_packet packeter;

  std::ifstream in("test_meteor_stream.s", std::ios::binary);
  std::vector<char> buffer((std::istreambuf_iterator<char>(in)),
                           (std::istreambuf_iterator<char>()));
  std::cout << "Raw Size " << buffer.size() << std::endl;

  uint8_t *raw = reinterpret_cast<uint8_t *>(buffer.data());
  uint8_t *ecced_data = new uint8_t[gr::starcoder::HARD_FRAME_LEN];

  int total = 0;
  int ok = 0;
  while (decoder.pos_ < buffer.size() - gr::starcoder::SOFT_FRAME_LEN) {
    total++;
    bool res = decoder.decode_one_frame(raw, ecced_data);
    if (res) {
      ok++;
      std::cout << std::dec << 100. * decoder.pos_ / buffer.size() << "% "
                << decoder.prev_pos_ << " " << std::hex << decoder.last_sync_
                << std::endl;
      packeter.parse_cvcdu(ecced_data, gr::starcoder::HARD_FRAME_LEN - 4 - 128);
    }
  }

  std::cout << std::dec << "packets: " << ok << " out of " << total
            << std::endl;

  CPPUNIT_ASSERT_EQUAL(ok, 58);
  CPPUNIT_ASSERT_EQUAL(total, 60);

  std::string png_img = packeter.dump_image();
  std::ifstream t("test_meteor_image.png", std::ios::binary);
  std::stringstream buf;
  buf << t.rdbuf();
  CPPUNIT_ASSERT_EQUAL(buf.str(), png_img);

  png_img = packeter.dump_gray_image(RED_APID);
  t = std::ifstream("test_meteor_image_68.png", std::ios::binary);
  buf = std::stringstream();
  buf << t.rdbuf();
  CPPUNIT_ASSERT_EQUAL(buf.str(), png_img);

  png_img = packeter.dump_gray_image(GREEN_APID);
  t = std::ifstream("test_meteor_image_65.png", std::ios::binary);
  buf = std::stringstream();
  buf << t.rdbuf();
  CPPUNIT_ASSERT_EQUAL(buf.str(), png_img);

  png_img = packeter.dump_gray_image(BLUE_APID);
  t = std::ifstream("test_meteor_image_64.png", std::ios::binary);
  buf = std::stringstream();
  buf << t.rdbuf();
  CPPUNIT_ASSERT_EQUAL(buf.str(), png_img);

  delete[] ecced_data;
}

} /* namespace starcoder */
} /* namespace gr */
