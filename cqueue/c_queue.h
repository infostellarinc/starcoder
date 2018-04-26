/*
 * Starcoder - a server to read/write data from/to the stars, written in Go.
 * Copyright (C) 2018 InfoStellar, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* This header can be read by both C and C++ compilers */
#ifndef C_QUEUE_H
#define C_QUEUE_H
#include <mutex>
#include <string>
#include <queue>
class c_queue {
  public:
    c_queue();
    void push(std::string);
    std::string pop();
    unsigned long get_ptr();
  private:
    std::queue<std::string> queue_;
    std::mutex mutex_;
};
c_queue *c_queue_from_ptr(unsigned long ptr);
#endif /*C_QUEUE_H*/
