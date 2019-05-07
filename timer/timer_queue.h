#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include <list>
#include <timer_tick.h>
#include <my_mutex.h>

namespace timer_tick
{

class TimerQueue
{
public:
  TimerQueue()
  {
  }

  ~TimerQueue(){}

  void add_timer(Timer* new_timer)
  {
    my_mutex::MutexLockGuard mlg(mutex_);
    if(timer_queue_.empty())
    {
      timer_queue_.push_back(new_timer);
      new_timer->set_iter(timer_queue_.begin());
      return;
    }

    for(auto iter = timer_queue_.begin(); iter != timer_queue_.end(); ++iter)
    {
      if(new_timer->overtime() < (*iter)->overtime())
      {
        auto new_iter = timer_queue_.insert(iter, new_timer);
        new_timer->set_iter(new_iter);
        return;
      }
    }
    auto new_iter = timer_queue_.insert(timer_queue_.end(), new_timer);
    new_timer->set_iter(new_iter);
  }

  void del_timer(Timer* del_timer)
  {
    my_mutex::MutexLockGuard mlg(mutex_);
    timer_queue_.erase(del_timer->iter());
  }

  Timer* top()
  {
    my_mutex::MutexLockGuard mlg(mutex_);
    if(timer_queue_.empty())
      return nullptr;
    else
      return *(timer_queue_.begin());
  }

  void pop()
  {
    my_mutex::MutexLockGuard mlg(mutex_);
    if(!timer_queue_.empty())
    {
      timer_queue_.erase(timer_queue_.begin());
    }
  }

  bool empty()
  {
    my_mutex::MutexLockGuard mlg(mutex_);
    return timer_queue_.empty();
  }

  int size()
  {
    my_mutex::MutexLockGuard mlg(mutex_);
    return timer_queue_.size();
  }

private:
  std::list<Timer*> timer_queue_;
  my_mutex::MutexLock mutex_;
};


} // namespace timer_tick


#endif // TIMER_QUEUE_H_