/**
 * @file TcpServer.h
 * @author Kingsley
 * @brief Pure Virtual Class
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <memory>
#include "parameters.h"

namespace http_server
{

class ThreadPool;

class Socket;

class TcpServer
{
public:
  TcpServer(ThreadPool* thread_pool, int listen_port);

  virtual ~TcpServer()
  {
  }

  virtual void handle_request() = 0;

  virtual void add_event(int fd, int event_type) = 0;

  virtual void del_event(int fd, int event_type) = 0;

  status add_task_to_pool(std::function<void ()> new_job);

  virtual void client_service(int client_fd) = 0;

  void setNoBlock(int fd);

protected:
  ThreadPool* thread_pool_;
  std::shared_ptr<Socket> socket_;

};

} // namespace http_server


#endif // TCP_SERVER_H_