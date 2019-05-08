/**
 * @file timer_tick.h
 * @author Kingsley
 * @brief Timer class. The timer contain overtime, callback function, iterator in queue and fd.
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef TIMER_TICK_H_
#define TIMER_TICK_H_

#include <functional>
#include <time.h>
#include <list>

namespace timer_tick
{

class Timer
{
public:
  typedef std::function<void (Timer*)> callback_func_;
  Timer(int fd, callback_func_ func, time_t overtime = 0) 
    : fd_(fd), 
      overtime_callback_(func), 
      overtime_(overtime)
  {
  }

  ~Timer(){}

  void set_callback_func(callback_func_ func)
  {
    overtime_callback_ = func;
  }

  void overtime_callback(Timer* timer)
  {
    if(overtime_callback_)
      overtime_callback_(timer);
  }

  void set_overtime(time_t overtime)
  {
    overtime_ = overtime;
  }

  time_t overtime()
  {
    return overtime_;
  }

  bool operator<(const Timer& b)
  {
    if(overtime_ < b.overtime_)
      return true;
    else
      return false; 
  }

  void set_iter(std::list<Timer*>::iterator iter)
  {
    iter_ = iter;
  }

  std::list<Timer*>::iterator iter()
  {
    return iter_;
  }

  int fd()
  {
    return fd_;
  }

private:
  int fd_;
  callback_func_ overtime_callback_;
  time_t overtime_;
  std::list<Timer*>::iterator iter_;
};


} // namesapce timer_tick



#endif  // TIMER_TICK_H_