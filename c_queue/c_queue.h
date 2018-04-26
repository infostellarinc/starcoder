/* This header can be read by both C and C++ compilers */
#ifndef C_QUEUE_H
#define C_QUEUE_H
#ifdef __cplusplus
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
#else
typedef
  struct c_queue
    c_queue;
#endif
#ifdef __cplusplus
extern "C" {
#endif
extern c_queue *c_queue_init();
extern c_queue *c_queue_from_ptr(unsigned long ptr);
extern unsigned long c_queue_get_ptr(c_queue *q);
extern const char *c_queue_pop(c_queue *q);
#ifdef __cplusplus
}
#endif
#endif /*C_QUEUE_H*/
