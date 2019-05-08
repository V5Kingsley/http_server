/**
 * @file main.cpp
 * @author Kingsley
 * @brief Http Server Main Cpp
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
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
  pool.close_pool();
}