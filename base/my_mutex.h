/**
 * @file my_mutex.h
 * @author Kingsley
 * @brief RAII Mutex
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef MY_MUTEX_H_
#define MY_MUTEX_H_
#include <boost/noncopyable.hpp>
#include <pthread.h>

namespace my_mutex{

class MutexLock : boost::noncopyable
{
public:
  MutexLock()
  {
    pthread_mutex_init(&mutex_, NULL);
  }

  ~MutexLock()
  {
    pthread_mutex_destroy(&mutex_);
  }

  void Lock()
  {
    pthread_mutex_lock(&mutex_);
  }

  void Unlock()
  {
    pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t* getPthreadMutex(){ return &mutex_; }

private:
  pthread_mutex_t mutex_;
};

class MutexLockGuard : boost::noncopyable
{
public:
  explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex)
  {
    mutex_.Lock();
  }

  ~MutexLockGuard()
  {
    mutex_.Unlock();
  }

private:
  MutexLock& mutex_;
};

}  //MyMutex

#endif
