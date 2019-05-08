/**
 * @file work_queue.h
 * @author Kingsley
 * @brief Work Queue For Thread Safety
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_

#include <my_mutex.h>
using namespace my_mutex;

template <class T>
struct work_node
{
  work_node *next;
  T work;
};

template <class T>
class WorkQueue
{
private:
  work_node<T> *head_node;
  work_node<T> *cur;
  MutexLock mutex;
  boost::atomic_int queue_size_; // Record the queue size. Can be replaced to int.

public:
  WorkQueue() : queue_size_(0)
  {
    head_node = new work_node<T>;
    head_node->next = nullptr;
    cur = head_node;
  }

  void push_work(T work)
  {
    MutexLockGuard mlg(mutex);
    work_node<T> *new_node = new work_node<T>;
    new_node->work = work;
    new_node->next = nullptr;
    cur->next = new_node;
    cur = new_node;
    queue_size_++;
  }

  bool empty()
  {
    MutexLockGuard mlg(mutex);
    if(cur == head_node)
      return true;
    else
      return false;
    
  }

  /*void pop_work()
  {
    MutexLockGuard mlg(mutex);
    if(cur == head_node)
      return;
    work_node<T> *temp = head_node->next;
    if(cur == temp)
      cur = head_node;
    head_node->next = temp->next;
    delete temp;
  }*/

/**
 * @brief Return the top work in queue and pop it.
 * 
 * @return T 
 */
  T pop_work()
  {
    MutexLockGuard mlg(mutex);
    if(cur == head_node)
      return nullptr;
    work_node<T> *temp = head_node->next;
    if(cur == temp)
      cur = head_node;
    head_node->next = temp->next;
    T work_to_pop = temp->work;
    delete temp;
    queue_size_--;
    return work_to_pop;
  }

  T top()
  {
    MutexLockGuard mlg(mutex);
    return head_node->next->work;
  }

  int size()
  {
    /*MutexLockGuard mlg(mutex);
    work_node<T> *pNode = head_node->next;
    int size = 0;
    while(pNode!=nullptr)
    {
      size++;
      pNode = pNode->next;
    }
    return size;*/
    int size = static_cast<int>(queue_size_);
    return size;
  }

  ~WorkQueue()
  {
    while(head_node->next!=nullptr)
    {
      work_node<T> *delNode = head_node->next;
      head_node->next = delNode->next;
      delete delNode;
    }
    delete head_node;
  }
};

#endif