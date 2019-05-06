#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <memory>

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

  void add_task_to_pool(std::function<void ()> new_job);

  virtual void client_service(int client_fd) = 0;

  void setNoBlock(int fd);

protected:
  ThreadPool* thread_pool_;
  std::shared_ptr<Socket> socket_;

};

} // namespace http_server


#endif // TCP_SERVER_H_