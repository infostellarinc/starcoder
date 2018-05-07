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

#include "blocking_spsc_queue.h"
#include <chrono>

blocking_spsc_queue::blocking_spsc_queue(int buffer_size) :
  queue_(buffer_size)
{ }

size_t blocking_spsc_queue::push(const char *arr, size_t size) {
  int pushed = queue_.push(arr, size);
  if (pushed > 0) {
    condition_var_.notify_one();
  }
  return pushed;
}

size_t blocking_spsc_queue::pop(char *arr, size_t size, int timeout_ms) {
  std::unique_lock<std::mutex> lock(mutex_);

  bool stat = condition_var_.wait_for(
    lock, std::chrono::milliseconds(timeout_ms),
    [this]{return !queue_.empty();}
  );

  if (!stat)
    // Timed out and queue is still empty.
    return 0;

  return queue_.pop(arr, size);
}

size_t blocking_spsc_queue_push(blocking_spsc_queue* q, const char *arr, size_t size) {
  return q->push(arr, size);
}

size_t blocking_spsc_queue_pop(blocking_spsc_queue* q, char *arr, size_t size, int timeout_ms) {
  return q->pop(arr, size, timeout_ms);
}
