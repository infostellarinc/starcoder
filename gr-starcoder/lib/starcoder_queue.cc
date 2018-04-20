#include "starcoder_queue.h"

starcoder_queue::starcoder_queue() { }

size_t starcoder_queue::push(const char *arr, size_t size) {
  return q_.push(arr, size);
}

size_t starcoder_queue::pop(char *arr, size_t size) {
  return q_.pop(arr, size);
}

size_t cpp_callback_push(starcoder_queue* q, const char *arr, size_t size) {
  return q->push(arr, size);
}

size_t cpp_callback_pop(starcoder_queue* q, char *arr, size_t size) {
  return q->pop(arr, size);
}
