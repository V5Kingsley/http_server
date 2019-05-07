#include <timer_tick.h>
#include <queue>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <timer_queue.h>

void callback(timer_tick::Timer* timer)
{}

/*struct Compare_timer
{
  bool operator()(timer_tick::Timer* a, timer_tick::Timer* b)
  {
    return a->overtime() > b->overtime();
  }
};

int main()
{
  std::priority_queue<timer_tick::Timer*, std::vector<timer_tick::Timer*>, Compare_timer> timers;
  for(int i = 0; i < 5; ++i)
  {
    timer_tick::Timer *new_timer = new timer_tick::Timer(callback, time(NULL));
    timers.push(new_timer);
    sleep(1);
  }

  timer_tick::Timer *new_timer = new timer_tick::Timer(callback, time(NULL)-10);
  timers.push(new_timer);

  while(!timers.empty())
  {
    printf("%ld\n", timers.top()->overtime());
    timers.pop();
  }

  return 0;
}*/

int main()
{
  timer_tick::TimerQueue timer_queue;


  timer_tick::Timer *new_timer1 = new timer_tick::Timer(1, callback, time(NULL)-10);
  timer_queue.add_timer(new_timer1);

  timer_tick::Timer *new_timer2 = new timer_tick::Timer(1, callback, time(NULL)-10);
  timer_queue.add_timer(new_timer2);


  timer_queue.del_timer(new_timer1);

  while(!timer_queue.empty())
  {
    printf("%ld\n", timer_queue.top()->overtime());
    timer_queue.pop();
  }
}