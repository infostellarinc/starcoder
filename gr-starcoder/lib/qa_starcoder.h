/* -*- c++ -*- */
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

#ifndef _QA_STARCODER_H_
#define _QA_STARCODER_H_

#include <gnuradio/attributes.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>

//! collect all the tests for the gr-filter directory
class __GR_ATTR_EXPORT qa_starcoder : public CppUnit::TestFixture
{
 public:
  //! return suite of tests for all of gr-filter directory
  static CppUnit::TestSuite *suite();
  void run_ar2300_source_block();

  /// Setup method
  void setUp() {}

  /// Teardown method
  void tearDown() {}
};

#endif /* _QA_STARCODER_H_ */
