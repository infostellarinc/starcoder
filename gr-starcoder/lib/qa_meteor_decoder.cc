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

#include "meteor/meteor_decoder.h"
#include "meteor/meteor_packet.h"

namespace gr {
namespace starcoder {

void qa_meteor_decoder::test_dump_empty() {
  meteor::packeter packeter;

  std::string empty;
  CPPUNIT_ASSERT_EQUAL(packeter.dump_image(), empty);
  CPPUNIT_ASSERT_EQUAL(packeter.dump_gray_image(meteor::RED_APID), empty);
}

void qa_meteor_decoder::test_full_decoding() {
  meteor::decoder decoder;
  meteor::packeter packeter;

  std::ifstream in("test_meteor_stream.s", std::ios::binary);
  std::vector<char> buffer((std::istreambuf_iterator<char>(in)),
                           (std::istreambuf_iterator<char>()));
  std::cout << "Raw Size " << buffer.size() << std::endl;

  uint8_t *raw = reinterpret_cast<uint8_t *>(buffer.data());
  std::unique_ptr<uint8_t[]> u_ecced_data(
      new uint8_t[meteor::HARD_FRAME_LEN]());
  uint8_t *ecced_data = u_ecced_data.get();

  int total = 0;
  int ok = 0;
  while (decoder.pos() < buffer.size() - meteor::SOFT_FRAME_LEN) {
    total++;
    bool res = decoder.decode_one_frame(raw, buffer.size(), ecced_data);
    if (res) {
      ok++;
      std::cout << std::dec << 100. * decoder.pos() / buffer.size() << "% "
                << decoder.prev_pos() << " " << std::hex << decoder.last_sync()
                << std::endl;
      packeter.parse_cvcdu(ecced_data, meteor::HARD_FRAME_LEN - 4 - 128);
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

  png_img = packeter.dump_gray_image(meteor::RED_APID);
  t = std::ifstream("test_meteor_image_68.png", std::ios::binary);
  buf = std::stringstream();
  buf << t.rdbuf();
  CPPUNIT_ASSERT_EQUAL(buf.str(), png_img);

  png_img = packeter.dump_gray_image(meteor::GREEN_APID);
  t = std::ifstream("test_meteor_image_65.png", std::ios::binary);
  buf = std::stringstream();
  buf << t.rdbuf();
  CPPUNIT_ASSERT_EQUAL(buf.str(), png_img);

  png_img = packeter.dump_gray_image(meteor::BLUE_APID);
  t = std::ifstream("test_meteor_image_64.png", std::ios::binary);
  buf = std::stringstream();
  buf << t.rdbuf();
  CPPUNIT_ASSERT_EQUAL(buf.str(), png_img);
}

} /* namespace starcoder */
} /* namespace gr */
