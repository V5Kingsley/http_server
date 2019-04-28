#include <my_thread.h>

namespace my_thread
{

boost::atomic_int Thread::thread_num(0);

void Thread::start()
{
  if (!started_)
  {
    pthread_create(&id_, NULL, thread_func<Thread>, this);
    started_ = true;
  }
  else
    return;
}

void Thread::join()
{
  if (started_ && !joined_)
    pthread_join(id_, NULL);
  return;
}

void Thread::detach()
{
  if (started_ && !detached_ && !joined_)
    pthread_detach(id_);
  else
    return;
}

} // namespace my_thread
