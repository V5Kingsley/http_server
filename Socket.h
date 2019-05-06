#ifndef SOCKET_H_
#define SOCKET_H_

#include <boost/noncopyable.hpp>

namespace http_server
{

class Socket : boost::noncopyable
{
public:
  explicit Socket(int listen_port);

  Socket() = delete;

  int fd() const { return sockfd_; }

  void set_reuseaddr();

  void bind();

  void listen();

  int accept();

  void close();

private:
  int listen_port_;
  int sockfd_;
};

}  // namespace http_server

#endif  // SOCKET_H_