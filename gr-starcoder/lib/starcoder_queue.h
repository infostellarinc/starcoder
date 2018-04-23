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
#include <boost/circular_buffer.hpp>
#include <mutex>
#include <condition_variable>
  typedef std::queue<char, boost::circular_buffer<char>> internal_ring_buffer;
  class starcoder_queue {
  public:
    starcoder_queue(int buffer_size);
    size_t push(const char*, size_t);
    size_t pop(char*, size_t, int);
  private:
    internal_ring_buffer q_;
    std::condition_variable condition_var_;
    std::mutex mutex_;
  };
#else
  typedef
    struct starcoder_queue
      starcoder_queue;
#endif
#ifdef __cplusplus
extern "C" {
#endif
  extern size_t push_to_queue(starcoder_queue*, const char*, size_t);
  extern size_t pop_from_queue(starcoder_queue*, char*, size_t, int);
#ifdef __cplusplus
}
#endif
#endif /*STARCODER_QUEUE_H*/
