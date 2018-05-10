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

#include <gnuradio/attributes.h>
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/message_strobe.h>
#include <cppunit/TestAssert.h>
#include "qa_enqueue_message_sink.h"
#include <chrono>
#include <thread>
#include <starcoder/enqueue_message_sink.h>
#include <string_queue.h>
#include <stdio.h>

namespace gr {
namespace starcoder {

void qa_enqueue_message_sink::test_no_registered_queue() {
  gr::top_block_sptr tb = gr::make_top_block("top");
  // The following source block is needed even if we don't use its message.
  // This is because the GNURadio scheduler will not create a thread for
  // blocks that aren't connected to anything. Thus the following block is
  // needed to have enqueue_message_sink connected to something.
  gr::block_sptr src = gr::blocks::message_strobe::make(pmt::mp("in"), 1000);
  gr::starcoder::enqueue_message_sink::sptr op =
      gr::starcoder::enqueue_message_sink::make();

  gr::basic_block_sptr bb = op->to_basic_block();
  bb->_post(pmt::mp("in"), pmt::make_u8vector(10, 97));

  tb->msg_connect(src, pmt::mp("strobe"), op, pmt::mp("in"));
  tb->start();

  // This sleep is needed since GNURadio messages are handled
  // asynchronously by the GNURadio scheduler. Unfortunately, even the
  // tests for message sinks in the gnuradio repository itself uses sleeps.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  tb->stop();
  tb->wait();
}

void qa_enqueue_message_sink::test_registered_queue() {
  gr::top_block_sptr tb = gr::make_top_block("top");
  // The following source block is needed even if we don't use its message.
  // This is because the GNURadio scheduler will not create a thread for
  // blocks that aren't connected to anything. Thus the following block is
  // needed to have enqueue_message_sink connected to something.
  gr::block_sptr src = gr::blocks::message_strobe::make(pmt::mp("in"), 1000);
  gr::starcoder::enqueue_message_sink::sptr op =
      gr::starcoder::enqueue_message_sink::make();
  string_queue q(10);

  op->register_starcoder_queue(q.get_ptr());

  gr::basic_block_sptr bb = op->to_basic_block();
  bb->_post(pmt::mp("in"), pmt::make_u8vector(10, 97));  // 97 is ascii for 'a'
  bb->_post(pmt::mp("in"), pmt::make_u8vector(3, 98));   // 98 is ascii for 'b'

  tb->msg_connect(src, pmt::mp("strobe"), op, pmt::mp("in"));
  tb->start();

  // This sleep is needed since GNURadio messages are handled
  // asynchronously by the GNURadio scheduler. Unfortunately, even the
  // tests for message sinks in the gnuradio repository itself uses sleeps.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  tb->stop();
  tb->wait();

  // Check that the passed PMT blobs are accessible from the queue
  // in the expected serialized binary format.
  CPPUNIT_ASSERT_EQUAL(q.pop(), pmt::serialize_str(pmt::make_u8vector(10, 97)));
  CPPUNIT_ASSERT_EQUAL(q.pop(), pmt::serialize_str(pmt::make_u8vector(3, 98)));

  // Check that after retrieving all available messages, the queue
  // returns the empty string.
  std::string empty_string = "";
  CPPUNIT_ASSERT_EQUAL(q.pop(), empty_string);
  CPPUNIT_ASSERT_EQUAL(q.pop(), empty_string);
}

} /* namespace starcoder */
} /* namespace gr */
