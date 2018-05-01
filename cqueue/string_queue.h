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
template <class T>
class string_queue {
  public:
    string_queue(int buffer_size);
    void push(const T &str);
    // This form of pop function is exception-unsafe, but is okay since we are returning
    // a std::string. Its copy constructor throws only when the system has run out
    // of memory. https://accu.org/index.php/journals/444
    T pop();
    T block_pop();
    unsigned long get_ptr();
    void wake();
  private:
    boost::lockfree::spsc_queue<T> queue_;
    std::condition_variable condition_var_;
    std::mutex mutex_;
};

template <class T>
string_queue<T>::string_queue(int buffer_size) : queue_(buffer_size) {}

template <class T>
void string_queue<T>::push(const T &item) {
  bool pushed = queue_.push(item);
  if (pushed) {
    condition_var_.notify_one();
  }
}

template <class T>
T string_queue<T>::pop() {
  T a;
  queue_.pop(a);
  return a;
}

template <class T>
T string_queue<T>::block_pop() {
  T a;
  std::unique_lock<std::mutex> lock(mutex_);
  if (queue_.empty())
    condition_var_.wait(lock);
  // If woken up spuriously, will return an empty string.
  queue_.pop(a);
  return a;
}

template <class T>
void string_queue<T>::wake() {
  condition_var_.notify_one();
}

template <class T>
uint64_t string_queue<T>::get_ptr() {
  return reinterpret_cast<uint64_t>(this);
}
#endif /*STRING_QUEUE_H*/
