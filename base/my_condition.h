/**
 * @file my_condition.h
 * @author Kingsley
 * @brief RAII Condition
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef MY_CONDITION_H_
#define MY_CONDITION_H_

#include <my_mutex.h>
#include <boost/noncopyable.hpp>

using namespace my_mutex;

namespace my_condition
{
class Condition : boost::noncopyable
{
public:
  explicit Condition(MutexLock& mutex) : mutex_(mutex)
  {
    pthread_cond_init(&pcond_, NULL);
  }

  ~Condition()
  {
    pthread_cond_destroy(&pcond_);
  }

  void notify()
  {
    //MutexLockGuard lg(mutex_);
    pthread_cond_signal(&pcond_);
  }

  void notifyAll()
  {
    //MutexLockGuard lg(mutex_);
    pthread_cond_broadcast(&pcond_);
  }

  void wait()
  {
    MutexLockGuard lg(mutex_);
    pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
  }

  bool waitForSeconds(double seconds);

private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;

};

} // namespace my_condition


#endif  //MY_CONDITION_H_
