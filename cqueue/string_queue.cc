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

#include "string_queue.h"
#include <iostream>

string_queue::string_queue(int buffer_size)
    : queue_(buffer_size), closed_(false) {}

void string_queue::push(const std::string &item) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool pushed = queue_.push(item);
  lock.unlock();
  if (pushed) {
    condition_var_.notify_one();
  }
}

std::string string_queue::pop() {
  std::string a;
  queue_.pop(a);
  if (a.length() > 10485760) {
    std::cerr << "Popped large packet of length " << a.length()
              << " from string_queue::pop\n";
    return "";
  }
  return a;
}

std::string string_queue::blocking_pop() {
  std::string a;
  std::unique_lock<std::mutex> lock(mutex_);
  condition_var_.wait(lock, [this] {
    return (!queue_.empty() || closed_);
  });
  queue_.pop(a);
  if (a.length() > 10485760) {
    std::cerr << "Popped large packet of length " << a.length()
              << " from string_queue::blocking_pop\n";
    return "";
  }
  return a;
}

void string_queue::close() {
  std::unique_lock<std::mutex> lock(mutex_);
  closed_ = true;
  lock.unlock();
  condition_var_.notify_one();
}

bool string_queue::closed() {
  std::unique_lock<std::mutex> lock(mutex_);
  return closed_;
}

uint64_t string_queue::get_ptr() const {
  return reinterpret_cast<uint64_t>(this);
}

string_queue *string_queue::queue_from_pointer(unsigned long long ptr) {
  return reinterpret_cast<string_queue *>(ptr);
}
