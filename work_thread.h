#ifndef WORK_THREAD_H_
#define WORK_THREAD_H_

#include <boost/noncopyable.hpp>
#include <memory>

#include <my_thread.h>
#include <my_mutex.h>
#include <my_condition.h>
#include "work_queue.h"

#include <iostream>

using namespace my_mutex;
using namespace my_condition;

namespace http_server
{

enum thread_state{BOOTING, READY, IDLE, BUSY, QUIT};

namespace work_thread
{

class Work
{
public:
  typedef std::function<void ()> work_func;
  typedef std::shared_ptr<Work> WorkPtr;

public:
  Work(work_func work) : work_(work)
  { 
  }

  Work(){}

  void execute_work();

  static WorkPtr create_work(work_func work)
  {
    return WorkPtr(new Work(work));
  }

  ~Work(){}


private:
  work_func work_;

};

class WorkThread : boost::noncopyable
{
public:
  typedef std::shared_ptr<WorkThread> WorkThreadPtr;
  typedef std::function<void (void*)> ThreadFunc;
  thread_state state_;

public:
  WorkThread(ThreadFunc func) 
  : thread_func_(func),
    mutex_(),
    pcond_(mutex_),
    thread_(new my_thread::Thread(thread_func_, this))
  {
  }

  ~WorkThread() {}

  void start()
  {
    thread_->start();
    thread_id_ = thread_->thread_id();
  }

  pthread_t work_thread_id()
  {
    return thread_id_;
  }

  MutexLock& get_mutex()
  {
    return mutex_;
  }

  Condition& get_condition()
  {
    return pcond_;
  }

  inline void add_work(Work::WorkPtr new_work);

  inline Work::WorkPtr pop_work();

  inline bool work_empty();

private:
  ThreadFunc thread_func_;
  MutexLock mutex_;
  Condition pcond_;
  
  std::shared_ptr<my_thread::Thread> thread_; 
  pthread_t thread_id_;

  WorkQueue<Work::WorkPtr> work_queue_;

};

inline void WorkThread::add_work(Work::WorkPtr new_work)
{
  work_queue_.push_work(new_work);
}

inline Work::WorkPtr WorkThread::pop_work()
{
  Work::WorkPtr tmp = work_queue_.pop_work();
  return tmp;
}

inline bool WorkThread::work_empty()
{
  return work_queue_.empty();
}

} // namespace work_thread

} // namespace http_server
 
#endif // WORK_THREAD_H_