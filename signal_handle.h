#ifndef SIGNAL_HANDLE_H_
#define SIGNAL_HANDLE_H_

#include <signal.h>
#include <boost/noncopyable.hpp>

namespace http_server
{

class SignalHandle : boost::noncopyable
{
public:
  explicit SignalHandle(){}

  void set_sigpipe(){}

private:

};

} // http_server



#endif