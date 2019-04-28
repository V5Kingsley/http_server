#include "my_condition.h"
#include "my_mutex.h"
#include "my_thread.h"
#include <iostream>

my_mutex::MutexLock mutex;
my_condition::Condition condition(mutex);

pthread_mutex_t mutex_nomal = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_nomal = PTHREAD_COND_INITIALIZER;

void thread_routine()
{
  sleep(2);
  {
  my_mutex::MutexLockGuard lg(mutex);
  condition.notify();
  }
  ///pthread_mutex_lock(&mutex_nomal);
  ///pthread_cond_signal(&cond_nomal);
 /// pthread_mutex_unlock(&mutex_nomal);

  std::cout<<"thread_routine ready"<<std::endl;
}


int main()
{
  my_thread::Thread t(thread_routine);
  t.start();

  condition.wait();
  ///pthread_mutex_lock(&mutex_nomal);
  ///pthread_cond_wait(&cond_nomal, &mutex_nomal);
  ///pthread_mutex_unlock(&mutex_nomal);

  std::cout<<"main() know thread ready"<<std::endl;

  while(1)
  {
    sleep(1);
  }
  
}