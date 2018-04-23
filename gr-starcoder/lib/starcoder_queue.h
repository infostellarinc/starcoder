/* This header can be read by both C and C++ compilers */
#ifndef STARCODER_QUEUE_H
#define STARCODER_QUEUE_H
#ifdef __cplusplus
#include <queue>
#include <boost/fiber/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
  class starcoder_queue {
  public:
    starcoder_queue();
    size_t push(const char*, size_t);
    size_t pop(char*, size_t);
  private:
    std::queue<char> q_;
    boost::fibers::condition_variable_any condition_var_;
    boost::mutex mutex_;
  };
#else
  typedef
    struct starcoder_queue
      starcoder_queue;
#endif
#ifdef __cplusplus
extern "C" {
#endif
#if defined(__STDC__) || defined(__cplusplus)
  extern size_t cpp_callback_push(starcoder_queue*, const char*, size_t);
  extern size_t cpp_callback_pop(starcoder_queue*, char*, size_t);
#else
  //extern void c_function();        /* K&R style */
  //extern int cplusplus_callback_function();
#endif
#ifdef __cplusplus
}
#endif
#endif /*STARCODER_QUEUE_H*/
