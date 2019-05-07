#include "TcpEpollServer.h"
#include "parameters.h"
#include <algorithm>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/timerfd.h>

//#define LOGGER_DEBUG
#define LOGGER_WARN
#include <logger.h>

#include "Socket.h"

namespace http_server
{

static const int MAX_MMAP_LENGTH = 10000;

TcpEpollServer::TcpEpollServer(ThreadPool *pool, parameters::Parameters *parameters)
  : TcpServer(pool, parameters->getListenPort()),
    http_parameters_(parameters)
{
  document_root_ = parameters->getDocumentRoot();
  default_file_ = parameters->getDefaultFile();
  file_mmap();
 // efd_ = eventfd(0, 0);
  assert(efd_ != -1);
}

int TcpEpollServer::efd_ = eventfd(0, 0);

/**
 * @brief Map the file to memory.
 * 
 */
void TcpEpollServer::file_mmap()
{
  std::vector<std::string> file_lists = http_parameters_->getHttpFileLists();
  file_fd_lists_.resize(file_lists.size());

  for(int i = 0; i < file_lists.size(); ++i)
  {
    file_fd_lists_[i] = open(file_lists[i].c_str(), O_RDONLY);
    if(file_fd_lists_[i] == -1)
    {
      WARN("open file %s failed!\n", file_lists[i].c_str());
      continue;
    }
    char* file_ptr = (char *)mmap(NULL, MAX_MMAP_LENGTH, PROT_READ, MAP_SHARED, file_fd_lists_[i], 0);
    if((char*)-1 == file_ptr)
    {
      WARN("mmap failure\n");
    }
    else
    {
      http_file_[file_lists[i]] = file_ptr;
    }
  }
}

TcpEpollServer::~TcpEpollServer()
{
  for(int i = 0; i < file_fd_lists_.size(); ++i)
  {
    if(file_fd_lists_[i] != -1)
      close(file_fd_lists_[i]);
  }

  for(auto iter = http_file_.begin(); iter != http_file_.end(); ++iter)
  {
    munmap(iter->second, MAX_MMAP_LENGTH);
  }
}

void TcpEpollServer::sig_int_handle(int sig)
{
  uint64_t u = 1;
  ssize_t rc;
  rc = write(efd_, &u, sizeof(uint64_t));
  if(rc != sizeof(uint64_t))
    WARN("sig int eventfd write error");
}

/**
 * @brief add event to epoll fd
 * 
 * @param fd 
 * @param event_type 
 */
void TcpEpollServer::add_event(int fd, int event_type)
{
  epoll_event e;
  e.data.fd = fd;
  e.events = event_type;
  epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &e);
}

/**
 * @brief delete event from epoll fd
 * 
 * @param fd 
 * @param event_type 
 */
void TcpEpollServer::del_event(int fd, int event_type)
{
  epoll_event e;
  e.data.fd = fd;
  e.events = event_type;
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &e);
}

/**
 * @brief close fd
 * 
 * @param fd 
 */
void TcpEpollServer::close_client(int fd)
{
  client_timers_queue_.del_timer(client_fd_array_[fd]);
  delete client_fd_array_[fd];
  client_fd_array_[fd] = nullptr;
  close(fd);
}

/**
 * @brief Read data from client fd. Get the request and give response to client
 * 
 * @param client_fd 
 */
void TcpEpollServer::client_service(int client_fd)
{
  DEBUG("handling client request... client fd: %d\n", client_fd);

  char rcv_buffer[BUFSIZ];
  char method[METHOD_LEN];
  char url[URL_LEN];
  char version[VERSION_LEN];

  char if_close;  //if client close itself, service will get 0 return.
  int ret = recv(client_fd, &if_close, 1, MSG_PEEK); 
  if(ret == 0)
  {
    close_client(client_fd);
    DEBUG("client close itself\n");
    return;
  }

  int n;
  n = get_line(client_fd, rcv_buffer, BUFSIZ);
  DEBUG("%s\n", rcv_buffer);  //GET /index.html HTTP/1.0

  int i = 0, j = 0;
  while (rcv_buffer[j] != ' ' && i <= METHOD_LEN - 1 && j <= n) //get the method
  {
    method[i] = rcv_buffer[j];
    ++i;
    ++j;
  }
  method[i] = '\0';
  DEBUG("method: %s\n", method);

  i = 0;
	while( rcv_buffer[j] ==' ' && j <= n)
		j++;
	while( rcv_buffer[j] != ' ' && j <= n && i < URL_LEN-1)//get the URL
	{
		url[i] = rcv_buffer[j];
		i++;
		j++;
	}
	url[i] = '\0';
	DEBUG("url: %s\n", url);

  i = 0;
	while(rcv_buffer[j] == ' ' && j <= n)
		j++;
	while(rcv_buffer[j] != '\n' && i < VERSION_LEN-1)
	{
		version[i] = rcv_buffer[j];
		i++;
		j++;
	}
	version[i] = '\0';
	DEBUG("version: %s\n", version);

  if(strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		//can not under stand the request
		unimplemented(client_fd);
		close_client(client_fd);
		return;
	}

  if(strcasecmp(method, "GET") == 0)
	{
		//call GET method function
		doGetMethod(client_fd, url, version);
	}
	else if(strcasecmp(method, "POST") == 0)
	{
		//call POST function
    close_client(client_fd);
	}

}

/**
 * @brief handle request with epoll method
 * 
 */
void TcpEpollServer::handle_request()
{
  signal(SIGPIPE, SIG_IGN);  // ignore sigpipe
  signal(SIGINT, sig_int_handle);

  if(socket_->fd() == -1)
  {
    WARN("Create listen fd failed!\n");
    return;
  }

  epoll_fd_ = epoll_create(5);
  assert(epoll_fd_ != -1);

  int time_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  assert(time_fd != -1);
  struct itimerspec new_value;
  struct timespec now;
  uint64_t exp;
  ssize_t s;
  assert(clock_gettime(CLOCK_REALTIME, &now) != -1);
  new_value.it_value.tv_sec = 2;
  new_value.it_value.tv_nsec = now.tv_nsec;
  new_value.it_interval.tv_sec = 2;
  new_value.it_interval.tv_nsec = 0;
  assert(timerfd_settime(time_fd, 0, &new_value, NULL) != -1);
  bool timer_tick = false;

  add_event(socket_->fd(), EPOLLIN);
  add_event(efd_, EPOLLIN);
  add_event(time_fd, EPOLLIN);

  int overtime_ms = -1;
  bool run = true;
  epoll_event events[MAXEVENTS];
  while(run == true)
  {
    int ret = epoll_wait(epoll_fd_, events, MAXEVENTS, overtime_ms);
    if(ret < 0 && errno != EINTR)
    {
      WARN("epoll wait failed!\n");
      break;
    }

    for(int i = 0; i < ret; ++i)
    {
      if(events[i].data.fd == socket_->fd())   // a new client
      {
        int client_fd = socket_->accept();
        if(client_fd == -1)
        {
          continue;
        }
        setNoBlock(client_fd);
        
        add_event(client_fd, EPOLLIN); 
        
        timer_tick::Timer *new_timer = new timer_tick::Timer(
          client_fd, std::bind(&TcpEpollServer::client_overtime_cb, this, std::placeholders::_1), time(NULL) + CLIENT_LIFE_TIME);
        assert(client_fd < MAX_FD);
        client_fd_array_[client_fd] = new_timer;
        client_timers_queue_.add_timer(new_timer);

        DEBUG("accept a new client[%d]\n", client_fd);
      }
      else if(events[i].data.fd == time_fd)
      {
        s = read(time_fd, &exp, sizeof(uint64_t));
        assert(s == sizeof(uint64_t));
        timer_tick = true;
        DEBUG("timer tick!!!\n");
      }
      else if((events[i].data.fd == efd_ ) && (events[i].events & EPOLLIN)) 
      {
        INFO("Got a sigint signal. Exiting...\n");
        run = false;
      }
      else if(events[i].events & EPOLLIN) // a client send request
      {
        DEBUG("receive a request from client[%d]\n", events[i].data.fd);
        status r = 
          add_task_to_pool(std::bind(
            &TcpEpollServer::client_service, this, static_cast<int>(events[i].data.fd)));
        /*if(r == FAILED)
        {
          continue;  // How to handle overflowed task?
        }*/
        del_event(events[i].data.fd, EPOLLIN);
      }
      else if(events[i].events & EPOLLRDHUP) // a client close the fd
      {
        DEBUG("EPOLLRDHUP!\n");
        close_client(events[i].data.fd);
        del_event(events[i].data.fd, EPOLLIN);
      }
      else
      {
        WARN("unexpected things happened! \n");
      }
    }

    if(timer_tick)
    {
      time_t current_time = time(NULL);
      
      while(!client_timers_queue_.empty())
      {
        timer_tick::Timer* top_timer = client_timers_queue_.top();
        if(top_timer->overtime() < current_time)
        {
          top_timer->overtime_callback(top_timer);
        }
        else
        {
          break;
        }   
      }

      DEBUG("client queue size: %d\n", client_timers_queue_.size());

    }
  }

  socket_->close();

}

/**
 * @brief this funtion get a string from a socket fd.
 *        this string's  last non-void charater is '\n'(the /r/n will be translated to '\n')
 * @param sock 
 * @param buf 
 * @param size 
 * @return int 
 */
int TcpEpollServer::get_line(int sock, char *buf, int size)
{
  int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
	n = recv(sock, &c, 1, 0);
	if(n > 0)
	{
		if(c == '\r')
		{
			n = recv(sock, &c, 1, MSG_PEEK);
			if((n > 0) && (c == '\n'))
			 recv(sock, &c, 1, 0);
			else
			 c = '\n';
	   }
	   buf[i] = c;
	   i++;
	}
	else
		c = '\n';
	}
	buf[i] = '\0';
 
 return i;
}

/**
 * @brief Reply to client that the server can not under stand his request
 * 
 * @param client 
 */
void TcpEpollServer::unimplemented(int client)
{
  char buf[1024];

  sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, SERVER_STRING);
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "</TITLE></HEAD>\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
}

/**
 * @brief Reply to the GET request
 * 
 * @param client_fd 
 * @param url file path
 * @param version http version
 */
void TcpEpollServer::doGetMethod(int client_fd, char *url, char *version)
{
  char path[BUFSIZ];
  struct stat st;
  char *query_string = NULL;
  query_string = url;
  while( *query_string != '?' && *query_string != '\0')
		query_string++;
	if(*query_string == '?')
	{
		*query_string = '\0';
		query_string++;
	}
  sprintf(path, "%s%s", document_root_, url);		
  if(path[strlen(path)-1] == '/')
		strcat(path, default_file_);
  
  DEBUG("Finding the file: %s\n", path);

  if(stat(path, &st) == -1)
  {
    DEBUG("can not find the file: %s\n", path);
    close_client(client_fd);
    return;
  }
  else
  {
    if(S_ISDIR(st.st_mode))
    {
      strcat(path, "/");
      strcat(path, default_file_);
    }

    if(st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode &S_IXOTH)
		{
			//CGI server
			execute_cgi(client_fd, path, "GET", query_string); 				
			close_client(client_fd);
		}
    else
    {
      file_serve(client_fd, path);
      close_client(client_fd);
    }
  }
  
}

/**
 * @brief Reply 404 to client
 * 
 * @param client 
 */
void TcpEpollServer::not_found(int client)
{
  char buf[1024];

  DEBUG("send the 404 infomation!\n");

  sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, SERVER_STRING);
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "your request because the resource specified\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "is unavailable or nonexistent.\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
}

void TcpEpollServer::execute_cgi(int client, const char *path, const char *method, const char *query_string)
{
}

/**
 * @brief Send the file to client.
 * 
 * @param client_fd 
 * @param filename 
 */
void TcpEpollServer::file_serve(int client_fd, char * filename)
{
  char buffer[BUFSIZ];
  int n = 1;
  buffer[0] = 'A';
  buffer[1] = '\0';
  while(n > 0 && strcmp(buffer, "\n"))//get and discard headers
  {
    get_line(client_fd, buffer, sizeof(buffer));
    DEBUG("%s\n", buffer);
  }

  std::string file = filename;
  if(http_file_.find(filename) == http_file_.end())
  {
    WARN("can not open the file: %s\n", filename);
    not_found(client_fd);
    close_client(client_fd);
    return;
  }
  else
  {
    DEBUG("Now send the file\n");
    headers(client_fd);
    send_file(client_fd, file);
  }
}

/**
 * @brief Send the http header(200 OK) to client.
 * 
 * @param client 
 */
void TcpEpollServer::headers(int client)
{
  char buf[1024];

  strcpy(buf, "HTTP/1.0 200 OK\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  strcpy(buf, "connection:keep-alive\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  strcpy(buf, SERVER_STRING);
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
  strcpy(buf, "\r\n");
  send(client, buf, strlen(buf), MSG_NOSIGNAL);
}

/**
 * @brief Send the content of the file from mmap
 * 
 * @param client 
 * @param filename 
 */
void TcpEpollServer::send_file(int client, std::string filename)
{
  send(client, http_file_[filename], strlen(http_file_[filename]), MSG_NOSIGNAL);
}

void TcpEpollServer::client_overtime_cb(timer_tick::Timer* overtime_timer)
{
  close_client(overtime_timer->fd());
}

} // namespace http_server

