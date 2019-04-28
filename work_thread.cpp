#include "work_thread.h"

#define LOGGER_WARN
#include <logger.h>

namespace http_server
{
namespace work_thread
{

void Work::execute_work()
{
  if (work_ == nullptr)
  {
    WARN("No work to execute !!! ");
  }
  else
  {
    work_();
  }
}

} // namespace work_thread

} // namespace http_server