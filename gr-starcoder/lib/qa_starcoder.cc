/*
 * Copyright 2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * This class gathers together all the test cases for the gr-filter
 * directory into a single test suite.  As you create new test cases,
 * add them here.
 */

#include "qa_starcoder.h"
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/null_sink.h>
#include <starcoder/ar2300_source.h>
#include <cppunit/TestCaller.h>
#include <chrono>
#include <thread>
#include <stdio.h>
#include "qa_enqueue_message_sink.h"

CppUnit::TestSuite *qa_starcoder::suite() {
  CppUnit::TestSuite *s = new CppUnit::TestSuite("starcoder");
  s->addTest(gr::starcoder::qa_enqueue_message_sink::suite());
  s->addTest(new CppUnit::TestCaller<qa_starcoder>(
      "run_ar2300_source_block", &qa_starcoder::run_ar2300_source_block));

  return s;
}

void qa_starcoder::run_ar2300_source_block() {
  gr::top_block_sptr tb = gr::make_top_block("top");
  gr::starcoder::ar2300_source::sptr src = gr::starcoder::ar2300_source::make();
  gr::block_sptr dst = gr::blocks::null_sink::make(sizeof(gr_complex));

  tb->connect(src, 0, dst, 0);
  tb->start();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  tb->stop();
  tb->wait();
}