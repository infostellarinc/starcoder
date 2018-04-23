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

starcoder_queue::starcoder_queue(int buffer_size) :
  q_(internal_ring_buffer(boost::circular_buffer<char>(buffer_size)))
{ }

size_t starcoder_queue::push(const char *arr, size_t size) {
  boost::mutex::scoped_lock lock(mutex_);
  bool const was_empty = q_.empty();
  for (int i = 0; i < size; i++) {
    q_.push(arr[i]);
  }
  if (was_empty) {
    condition_var_.notify_one();
  }
  return size;
}

size_t starcoder_queue::pop(char *arr, size_t size) {
  boost::mutex::scoped_lock lock(mutex_);
  while (q_.empty()) {
    condition_var_.wait(lock);
  }

  int i = 0;
  while (i < size && !q_.empty()) {
    arr[i] = q_.front();
    q_.pop();
    i++;
  }
  return i;
}

size_t push_to_queue(starcoder_queue* q, const char *arr, size_t size) {
  return q->push(arr, size);
}

size_t pop_from_queue(starcoder_queue* q, char *arr, size_t size) {
  return q->pop(arr, size);
}
