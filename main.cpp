#include "parameters.h"
#include "thread_pool.h"

int main(int argc, char *argv[])
{
  http_server::parameters::Parameters parameters(argc, argv);
  parameters.displayConfig();
  http_server::ThreadPool pool(&parameters);
  pool.start();

  sleep(5);
}