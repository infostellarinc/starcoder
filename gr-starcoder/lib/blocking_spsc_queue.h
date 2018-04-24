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
#include <mutex>
#include <condition_variable>
#include <boost/lockfree/spsc_queue.hpp>
class blocking_spsc_queue {
  public:
    blocking_spsc_queue(int buffer_size);
    size_t push(const char*, size_t);
    size_t pop(char*, size_t, int);
  private:
    // Although we use the threadsafe spsc_queue here, we still need to keep the
    // mutex for the sole purpose of waiting for the queue to be non-empty.
    // If the pop method were non-blocking, GNURadio would call it as fast as the
    // CPU can process (consuming 100% CPU), even though the AR2300 only creates 1.125Ms/s.
    boost::lockfree::spsc_queue<char> queue_;
    std::condition_variable condition_var_;
    std::mutex mutex_;
};
#else
typedef
  struct blocking_spsc_queue
    blocking_spsc_queue;
#endif
#ifdef __cplusplus
extern "C" {
#endif
extern size_t blocking_spsc_queue_push(blocking_spsc_queue* q, const char* arr, size_t size);
extern size_t blocking_spsc_queue_pop(blocking_spsc_queue* q, char* arr, size_t size, int timeout_ms);
#ifdef __cplusplus
}
#endif
#endif /*STARCODER_QUEUE_H*/
