/**
 * @file parameters.h
 * @author Kingsley
 * @brief 
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <getopt.h>
#include <vector>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

using boost::property_tree::ptree;
using boost::property_tree::ptree_error;
using boost::property_tree::ptree_bad_path;

namespace http_server
{

enum status{SUCCESS, FAILED};
#define SERVER_STRING "Server:A http server by Kingsley\r\n"

namespace parameters
{
static const int MAX_FILE_LINE = 128;
#define LISTEN_PORT 54321
#define MAX_CLIENT 10000
#define TIME_OUT 10
#define INIT_WORKER_NUM 5
#define MAX_WORK_NUM 100000

/* the short cmd opt string */
static const char *short_cmd_opt = "c:d:f:o:l:m:t:i:w:h";

/*the long cmd opt structure*/
static struct option long_cmd_opt[] = {
    {"CGIRoot", required_argument, nullptr, 'c'},
    {"DefaultFile", required_argument, nullptr, 'd'},
    {"DocumentRoot", required_argument, nullptr, 'o'},
    {"ConfigFile", required_argument, nullptr, 'f'},
    {"ListenPort", required_argument, nullptr, 'l'},
    {"MaxClient", required_argument, nullptr, 'm'},
    {"TimeOut", required_argument, nullptr, 't'},
    {"InitWorkerNum", required_argument, nullptr, 'i'},
    {"MaxWorkNum", required_argument, nullptr, 'w'},
    {"help", no_argument, nullptr, 'h'},
};

class Parameters
{
public:
  Parameters(int argc, char *argv[]);
  void displayConfig();

  int getListenPort(){ return listen_port_; }

  int getInitWorkerNum() { return init_worker_num_; }

  int getMaxWorkNum() { return max_work_num_; }

  char* getDocumentRoot() { return document_root_; }

  char* getDefaultFile() { return default_file_; }

  std::vector<std::string> getHttpFileLists() { return file_lists_; }



private:
  bool loadConfig();
  char CGI_root_[MAX_FILE_LINE];
  char default_file_[MAX_FILE_LINE];
  char document_root_[MAX_FILE_LINE];
  char config_file_[MAX_FILE_LINE];
  int listen_port_;
  int max_client_;
  int time_out_;
  int init_worker_num_;
  int max_work_num_;
  std::vector<std::string> file_lists_;
  ptree xml_tree_;
};
}
} // namespace http_server

#endif // PARAMETERS_H_