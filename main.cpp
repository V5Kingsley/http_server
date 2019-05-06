#include "parameters.h"
#include "thread_pool.h"
#include "TcpEpollServer.h"

int main(int argc, char *argv[])
{
  http_server::parameters::Parameters parameters(argc, argv);
  parameters.displayConfig();
  http_server::ThreadPool pool(&parameters);
  pool.start();
  http_server::TcpEpollServer server(&pool, &parameters);
  server.handle_request();

  sleep(5);
}