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

#include "qa_command_source.h"
#include <cppunit/TestAssert.h>
#include <gnuradio/attributes.h>
#include <gnuradio/blocks/message_debug.h>
#include <gnuradio/top_block.h>
#include <starcoder/command_source.h>
#include <stdio.h>
#include <string_queue.h>
#include <chrono>
#include <thread>
#include "starcoder.pb.h"

namespace gr {
namespace starcoder {

void qa_command_source::test_() {
  gr::top_block_sptr tb = gr::make_top_block("top");

  gr::starcoder::command_source::sptr op =
      gr::starcoder::command_source::make();
  gr::block_sptr sink = gr::blocks::message_debug::make();

  ::starcoder::PMT pmt;

  tb->msg_connect(op, pmt::mp("out"), sink, pmt::mp("print"));
  tb->start();

  // This sleep is needed since GNURadio messages are handled
  // asynchronously by the GNURadio scheduler. Unfortunately, even the
  // tests for message sinks in the gnuradio repository itself uses sleeps.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  tb->stop();
  tb->wait();
}

} /* namespace starcoder */
} /* namespace gr */
