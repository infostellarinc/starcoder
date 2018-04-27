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

#include "c_queue.h"

c_queue::c_queue(int buffer_size) : queue_(buffer_size) {}

void c_queue::push(const std::string &item) {
  bool pushed = queue_.push(item);
  if (pushed) {
    condition_var_.notify_one();
  }
}

std::string c_queue::pop() {
  std::string a;
  queue_.pop(a);
  return a;
}

std::string c_queue::block_pop() {
  std::string a;
  std::unique_lock<std::mutex> lock(mutex_);
  condition_var_.wait(lock);
  // If woken up spuriously, will return an empty string.
  queue_.pop(a);
  return a;
}

void c_queue::wake() {
  condition_var_.notify_one();
}

uint64_t c_queue::get_ptr() {
  return reinterpret_cast<uint64_t>(this);
}

c_queue *c_queue_from_ptr(uint64_t ptr) {
    return reinterpret_cast<c_queue *>(ptr);
}
