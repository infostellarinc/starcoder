#include "starcoder_queue.h"

starcoder_queue::starcoder_queue() { }

size_t starcoder_queue::push(const char *arr, size_t size) {
  boost::mutex::scoped_lock lock(mutex_);
  bool const was_empty = q_.empty();
  for (int i=0; i < size; i++) {
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
    i ++;
  }
  return i;
}

size_t cpp_callback_push(starcoder_queue* q, const char *arr, size_t size) {
  return q->push(arr, size);
}

size_t cpp_callback_pop(starcoder_queue* q, char *arr, size_t size) {
  return q->pop(arr, size);
}
