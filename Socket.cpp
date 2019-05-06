#include "Socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <string.h>

namespace http_server
{
Socket::Socket(int listen_port)
  : listen_port_(listen_port),
    sockfd_(socket(AF_INET, SOCK_STREAM, 0))
{
}

/**
 * @brief Set socket addr reuse.
 * 
 */
void Socket::set_reuseaddr()
{
  int reuse = 1;
  assert(setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == 0);
}

void Socket::bind()
{
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(listen_port_);
  ::bind(sockfd_, (sockaddr*)&server_addr, sizeof(server_addr));
}

void Socket::listen()
{
  ::listen(sockfd_, 10000);
}

/**
 * @brief Accept client
 * 
 * @return int Client fd;
 */
int Socket::accept()
{
  return ::accept(sockfd_, NULL, 0);
}

/**
 * @brief Close fd.
 * 
 */
void Socket::close()
{
  ::close(sockfd_);
}


} // namespace http_server