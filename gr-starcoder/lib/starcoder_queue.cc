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

#include "starcoder_queue.h"
#include <chrono>

starcoder_queue::starcoder_queue(int buffer_size) :
  queue_(boost::circular_buffer<char>(buffer_size))
{ }

size_t starcoder_queue::push(const char *arr, size_t size) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool const was_empty = queue_.empty();
  for (int i = 0; i < size; i++) {
    queue_.push(arr[i]);
  }
  if (was_empty) {
    condition_var_.notify_one();
  }
  return size;
}

size_t starcoder_queue::pop(char *arr, size_t size, int timeout_ms) {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.empty()) {
    condition_var_.wait_for(lock, std::chrono::milliseconds(timeout_ms));
  }

  int popped = 0;
  while (popped < size && !queue_.empty()) {
    arr[popped] = queue_.front();
    queue_.pop();
    popped++;
  }
  return popped;
}

size_t push_to_queue(starcoder_queue* q, const char *arr, size_t size) {
  return q->push(arr, size);
}

size_t pop_from_queue(starcoder_queue* q, char *arr, size_t size, int timeout_ms) {
  return q->pop(arr, size, timeout_ms);
}
