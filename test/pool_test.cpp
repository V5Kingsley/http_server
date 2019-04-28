#include "../thread_pool.h"
#include "../parameters.h"
#include <logger.h>
#include <time.h>
using namespace http_server;

my_mutex::MutexLock mutex;

int cnt = 0;

int task(int num)
{
  my_mutex::MutexLockGuard mlg(mutex);
  //sleep(0.1);
  cnt++;
}

/*int main()
{
  http_server::ThreadPool pool;
  pool.setThreadNum(5);
  pool.start();
  for (int i = 0; i < 10; i++)
  {
    work_thread::WorkThread *this_work_thread = pool.get_next_work_thread();
    std::shared_ptr<work_thread::Work> new_work = work_thread::Work::create_work(std::bind(task, i));
    this_work_thread->add_work(new_work);
  }

  while (1)
  {
    sleep(5);
  }
}*/

int main(int argc, char **argv)
{
  http_server::parameters::Parameters parameters(argc, argv);
  parameters.displayConfig();

  http_server::ThreadPool pool(&parameters);

  int task_num = 100000;

  pool.start();

  clock_t start, stop;
  double durationTime;
  start = clock();
  int success_task = 0;
  for(int i = 0; i < task_num; ++i)
  {
    if(pool.add_task_to_pool(std::bind(task, i)) == http_server::SUCCESS)
      success_task++;
  }
  std::cout<<"success task to past num: "<<success_task<<std::endl;
  while(cnt != success_task)
  {
    sleep(0.1);
  }

  stop = clock();
  
  durationTime = ((double)(stop-start))/CLOCKS_PER_SEC;
  std::cout << "程序耗时：" << durationTime << " s" << std::endl;
  //sleep(5);

  INFO("total task: %d\n", static_cast<int>(cnt));
}