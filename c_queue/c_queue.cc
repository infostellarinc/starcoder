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

c_queue::c_queue() {}

void c_queue::push(std::string item) {
  std::unique_lock<std::mutex> lock(mutex_);
  queue_.push(item);
}

std::string c_queue::pop() {
  std::unique_lock<std::mutex> lock(mutex_);
  std::string a;
  if (!queue_.empty()) {
    a = queue_.front();
    queue_.pop();
  }
  return a;
}

unsigned long c_queue::get_ptr() {
  return reinterpret_cast<unsigned long>(this);
}

c_queue *c_queue_init() {
  c_queue *q = new c_queue();
  q->push("aaa");
  q->push("bbb");
  return q;
}

c_queue *c_queue_from_ptr(unsigned long ptr) {
  c_queue *q = reinterpret_cast<c_queue *>(ptr);
  return q;
}

const char *c_queue_pop(c_queue *q) {
  std::string a = q->pop();
  return a.c_str();
}

extern unsigned long c_queue_get_ptr(c_queue *q) {
  return q->get_ptr();
}