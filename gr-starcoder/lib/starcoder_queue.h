/* This header can be read by both C and C++ compilers */
#ifndef STARCODER_QUEUE_H
#define STARCODER_QUEUE_H
#ifdef __cplusplus
#include <boost/lockfree/spsc_queue.hpp>
  class starcoder_queue {
  public:
    starcoder_queue();
    size_t push(const char*, size_t);
    size_t pop(char*, size_t);
  private:
    boost::lockfree::spsc_queue<char, boost::lockfree::capacity<1048576> > q_;
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
