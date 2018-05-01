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
#ifndef STRING_QUEUE_H
#define STRING_QUEUE_H
#include <mutex>
#include <condition_variable>
#include <string>
#include <boost/lockfree/spsc_queue.hpp>
class string_queue {
  public:
    string_queue(int buffer_size);
    void push(const std::string &str);
    // This form of pop function is exception-unsafe, but is okay since we are returning
    // a std::string. Its copy constructor throws only when the system has run out
    // of memory. https://accu.org/index.php/journals/444
    std::string pop();
    std::string blocking_pop();
    unsigned long get_ptr();
    void wake();
  private:
    boost::lockfree::spsc_queue<std::string> queue_;
    std::condition_variable condition_var_;
    std::mutex mutex_;
};
string_queue *string_queue_from_ptr(uint64_t ptr);
#endif /*STRING_QUEUE_H*/
