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

/* This header can be read by both C and C++ compilers */
#ifndef STARCODER_QUEUE_H
#define STARCODER_QUEUE_H
#ifdef __cplusplus
#include <queue>
#include <boost/fiber/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
  class starcoder_queue {
  public:
    starcoder_queue();
    size_t push(const char*, size_t);
    size_t pop(char*, size_t);
  private:
    std::queue<char> q_;
    boost::fibers::condition_variable_any condition_var_;
    boost::mutex mutex_;
  };
#else
  typedef
    struct starcoder_queue
      starcoder_queue;
#endif
#ifdef __cplusplus
extern "C" {
#endif
#if defined(__STDC__) || defined(__cplusplus)
  extern size_t cpp_callback_push(starcoder_queue*, const char*, size_t);
  extern size_t cpp_callback_pop(starcoder_queue*, char*, size_t);
#else
  //extern void c_function();        /* K&R style */
  //extern int cplusplus_callback_function();
#endif
#ifdef __cplusplus
}
#endif
#endif /*STARCODER_QUEUE_H*/
