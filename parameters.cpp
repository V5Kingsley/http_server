/**
 * @file parameters.cpp
 * @author Kingsley
 * @brief 
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "parameters.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace http_server
{

namespace parameters
{
Parameters::Parameters(int argc, char *argv[])
    : CGI_root_("doc/cgi-bin/"),
      default_file_("index.html"),
      document_root_("doc"),
      config_file_("doc/config.xml"),
      listen_port_(LISTEN_PORT),
      max_client_(MAX_CLIENT),
      time_out_(TIME_OUT),
      init_worker_num_(INIT_WORKER_NUM),
      max_work_num_(MAX_WORK_NUM)
{
  loadConfig();
  if (argc >= 2 && argv != nullptr)
  {
    int c = -1, value = -1;
    while ((c = getopt_long(argc, argv, short_cmd_opt, long_cmd_opt, NULL)) != -1) //getopt_long：解析命令行参数
    {
      switch (c)
      {
      case 'c':
        printf("set CGIRoot: %s\n", optarg); //optarg 指向-c参数后面的元素
        strcpy(CGI_root_, optarg);
        break;
      case 'd':
        printf("set DefaultFile: %s\n", optarg);
        strcpy(default_file_, optarg);
        break;
      case 'o':
        printf("set DocumentRoot: %s\n", optarg);
        strcpy(document_root_, optarg);
        break;
      case 'f':
        printf("set ConfigFile: %s\n", optarg);
        strcpy(config_file_, optarg);
        break;
      case 'l':
        value = atoi(optarg); //atoi：把字符串转换成整形数
        printf("set ListenPort: %d\n", value);
        listen_port_ = value;
        break;
      case 'm':
        value = atoi(optarg);
        printf("set MaxClient: %d\n", value);
        max_client_ = value;
        break;
      case 't':
        value = atoi(optarg);
        printf("set TimeOut: %d\n", value);
        time_out_ = value;
        break;
      case 'i':
        value = atoi(optarg);
        printf("set InitWorkerNum: %d\n", value);
        init_worker_num_ = value;
        break;
      case 'w':
        value = atoi(optarg);
        printf("set MaxWorkerNum: %d\n", value);
        max_work_num_ = value;
        break;
      case 'h':
        printf("help test");
        break;
      default:
        break;
      }
    }
  }
  char path[BUFSIZ];
  sprintf(path, "%s%s%s", document_root_, "/", default_file_);
  file_lists_.push_back(path);
}

void Parameters::displayConfig()
{
  //printf("http sever CGIRoot: %s\n", CGI_root_);
  printf("http sever DefaultFile: %s\n", default_file_);
  printf("http sever DocumentRoot: %s\n", document_root_);
  //printf("http sever ConfigFile: %s\n", config_file_);
  printf("http sever ListenPort: %d\n", listen_port_);
  //printf("http sever MaxClient: %d\n", max_client_);
  //printf("http sever TimeOut: %d\n", time_out_);
  printf("http sever InitWorkerNum: %d\n", init_worker_num_);
  printf("http server MaxWorkNum: %d\n", max_work_num_);
}

bool Parameters::loadConfig()
{
  try
  {
    boost::property_tree::read_xml("doc/config.xml", xml_tree_, boost::property_tree::xml_parser::trim_whitespace);
  }
  catch (const ptree_error &e)
  {
    printf("read xml file error: %s\n", e.what());
    return false;
  }

  try
  {
    int listen_port = xml_tree_.get_child("root.http_server.listen_port").get<int>("<xmlattr>.value");
    listen_port_ = listen_port;
  }
  catch (const ptree_error &e)
  {
    printf("read xml listen_port error: %s\n", e.what());
  }

  try
  {
    int init_worker_num = xml_tree_.get_child("root.http_server.init_worker_num").get<int>("<xmlattr>.value");
    init_worker_num_ = init_worker_num;
  }
  catch (const ptree_error &e)
  {
    printf("read xml init_worker_num error: %s\n", e.what());
  }

  try
  {
    int max_work_num = xml_tree_.get_child("root.http_server.max_work_num").get<int>("<xmlattr>.value");
    max_work_num_ = max_work_num;
  }
  catch (const ptree_error &e)
  {
    printf("read xml max_work_num error: %s\n", e.what());
  }

  try
  {
    std::string document_root = xml_tree_.get_child("root.http_server.document_root").get<std::string>("<xmlattr>.value");
    memset(document_root_, 0, sizeof(document_root_));
    strncpy(document_root_, document_root.c_str(), sizeof(document_root_) - 1);
    document_root_[MAX_FILE_LINE - 1] = '\0';
  }
  catch (const ptree_error &e)
  {
    printf("read xml document_root: %s\n", e.what());
  }

  try
  {
    std::string default_file = xml_tree_.get_child("root.http_server.default_file").get<std::string>("<xmlattr>.value");
    memset(default_file_, 0, sizeof(default_file_));
    strncpy(default_file_, default_file.c_str(), sizeof(default_file_) - 1);
    default_file_[MAX_FILE_LINE - 1] = '\0';
  }
  catch (const ptree_error &e)
  {
    printf("read xml default_file: %s\n", e.what());
  }
  
}


}
} // namespace http_server