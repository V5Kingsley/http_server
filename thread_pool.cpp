#include "thread_pool.h"
#include <signal.h>

//#define LOGGER_DEBUG
#define LOGGER_WARN
#include <logger.h>

namespace http_server
{

static struct timeval delay = {0, 2}; 

ThreadPool::ThreadPool(parameters::Parameters *pool_parameters)
  : next_(0),
    threads_num_(0),
    started(false),
    boot_mutex_(),
    boot_cond_(boot_mutex_),
    pool_activate(false),
    pool_parameters_(pool_parameters)
{
  threads_num_ = pool_parameters_->getInitWorkerNum();
  max_work_num_ = pool_parameters_->getMaxWorkNum();
}

/**
 * @brief Start the pool. Set up the work threads.
 * 
 */
void ThreadPool::start()
{
  assert(!started);
  assert(threads_num_ != 0);

  sem_init(&task_num_, 0, 0);

  started = true;

  pthread_barrier_init(&pool_barrier_, NULL, threads_num_ + 1);

  for(int i = 0; i < threads_num_; ++i)
  {
    work_thread::WorkThread* t = new work_thread::WorkThread(std::bind(&ThreadPool::thread_routine, this, i));
    work_threads_.push_back(t);
    t->state_ = BOOTING;
    t->start();
    boot_cond_.wait();
    t->state_ = READY;
  }

  pool_activate = true;

  distribute_thread_ = std::shared_ptr<my_thread::Thread>(new my_thread::Thread(std::bind(&ThreadPool::distribute_task, this)));
  distribute_thread_->start();

  pthread_barrier_wait(&pool_barrier_);

  INFO("Thread pool is ready to work.\n");  
}

/**
 * @brief Get the next work thread to assign task by round-robin algorithm
 * 
 * @return work_thread::WorkThread* 
 */
work_thread::WorkThread* ThreadPool::get_next_work_thread()
{
  work_thread::WorkThread* next_work_thread;
  next_work_thread = work_threads_[next_];
  ++next_;
  if(static_cast<size_t>(next_) >= work_threads_.size())
  {
    next_ = 0;
  }
  return next_work_thread;
}

/**
 * @brief Work thread function. Get work from queue to execute.
 * 
 * @param index Work thread index in vector.
 */
void ThreadPool::thread_routine(int index)
{
  pthread_detach(pthread_self());
  INFO("Generated a work thread. thread id: %lu\n", pthread_self())

  assert(index < work_threads_.size());
  work_thread::WorkThread* this_work_thread = work_threads_[index];

  while(this_work_thread->state_ == BOOTING)
  {
    my_mutex::MutexLockGuard mlg(boot_mutex_);
    boot_cond_.notify();
  }

  pthread_barrier_wait(&pool_barrier_);

  while(pool_activate)
  {
    if(this_work_thread->work_empty())
    {
      {
        my_mutex::MutexLockGuard mlg(this_work_thread->get_mutex());
        this_work_thread->state_ = IDLE;
      }
      this_work_thread->get_condition().wait();
    }
    if(pool_activate == false)
      break;
    while(!this_work_thread->work_empty())
    {
      DEBUG("get a job. thread id: %lu\n", pthread_self());
      (this_work_thread->pop_work())->execute_work();
    }
  }
  INFO("Work thread %d exits.\n", index + 1);
}

/**
 * @brief Distribute task to work thread with round-robin algorithm.
 * 
 */
void ThreadPool::distribute_task()
{
  pthread_detach(pthread_self());
  INFO("distribute_task function started. thread id: %lu\n", pthread_self())

  while(pool_activate)
  {
    sem_wait(&task_num_);
    if(pool_activate != true)
      break;
    work_thread::WorkThread* selected_thread = get_next_work_thread();
    assert(!pool_work_queue_.empty());
    work_thread::Work::WorkPtr work_to_past = pool_work_queue_.pop_work();
    selected_thread->add_work(work_to_past);
    {
      my_mutex::MutexLockGuard mlg(selected_thread->get_mutex());
      if(selected_thread->state_ == IDLE)
      {
        selected_thread->state_ = BUSY;
        selected_thread->get_condition().notify();
      }
    }
  }
  INFO("Distrubute task thread exits.\n");
}

/**
 * @brief Add task to queue. Controled by Semaphore.
 * 
 * @param new_task 
 * @return status 
 */
status ThreadPool::add_task_to_pool(TaskFunc new_task)
{
  if(pool_work_queue_.size() > max_work_num_)
  {
    WARN("Thread pool is busy. queue size: %d\n", pool_work_queue_.size());
    return FAILED;
  }
  std::shared_ptr<work_thread::Work> new_work = work_thread::Work::create_work(new_task);
  pool_work_queue_.push_work(new_work);
  //my_mutex::MutexLockGuard mlg(pool_mutex_);
  sem_post(&task_num_);
  return SUCCESS;
}

void ThreadPool::close_pool()
{
  pool_activate = false;
  sem_post(&task_num_);
  while(pthread_kill(distribute_thread_->thread_id(), 0) == 0)
  {
    select(0, NULL, NULL, NULL, &delay);
  }
  sem_destroy(&task_num_);

  for (int i = 0; i < work_threads_.size(); ++i)
  {
    while (pthread_kill(work_threads_[i]->work_thread_id(), 0) == 0)
    {
      work_threads_[i]->get_condition().notify();
      select(0, NULL, NULL, NULL, &delay);
    }
  }
  INFO("Thread pool is closed successfully.\n");
}

} // namespace http_server