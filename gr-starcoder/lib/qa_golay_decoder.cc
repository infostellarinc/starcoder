/* -*- c++ -*- */
/*
 * Copyright 2018 Infostellar.
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

#include "qa_golay_decoder.h"

#include <chrono>
#include <cppunit/TestAssert.h>
#include <gnuradio/attributes.h>
#include <gnuradio/blocks/message_debug.h>
#include <gnuradio/top_block.h>
#include <starcoder/golay_decoder.h>
#include <thread>

namespace gr {
namespace starcoder {

void qa_golay_decoder::t1() {
  gr::top_block_sptr tb = gr::make_top_block("qa_golay_decoder");
  golay_decoder::sptr decoder = golay_decoder::make(10, 1);
  gr::blocks::message_debug::sptr snk = gr::blocks::message_debug::make();
  tb->msg_connect(decoder, pmt::mp("out"), snk, pmt::mp("store"));

  tb->start();

  decoder->_post(pmt::mp("in"),
                 pmt::cons(pmt::make_dict(), pmt::make_u8vector(1000, 0x25)));

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  tb->stop();
  tb->wait();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  CPPUNIT_ASSERT_EQUAL(1, snk->num_messages());
  CPPUNIT_ASSERT(
      pmt::equal(pmt::cons(pmt::PMT_NIL, pmt::make_u8vector(976, 0x25)),
                 snk->get_message(0)));
}

} /* namespace starcoder */
} /* namespace gr */
