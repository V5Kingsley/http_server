/**
 * @file logger.h
 * @author Kingsley
 * @brief logger class. INFO WARN DEBUG. 
 *        If use WARN and DEBUG, must call #define LOGGER_WARN and #define LOGGER_DEBUG before include the file.
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include "pthread.h"

/**
 * @brief logger mutex singleton class. 
 * 
 */
class LoggerMutex
{
public:
  static LoggerMutex *logger_mutex;
  void lock()
  {
    pthread_mutex_lock(&mutex_);
  }
  void unlock()
  {
    pthread_mutex_unlock(&mutex_);
  }
  
private:
  LoggerMutex(const LoggerMutex &) {}
  LoggerMutex()
  {
    pthread_mutex_init(&mutex_, NULL);
  }
  LoggerMutex &operator=(const LoggerMutex &) {}
  pthread_mutex_t mutex_;
};


#ifdef LOGGER_DEBUG
#define DEBUG(arg...) \
  {                   \
    LoggerMutex::logger_mutex->lock(); \
    printf("\033[32m[DEBUG]"); \
    printf(arg);      \
    printf("\033[0m");  \
    LoggerMutex::logger_mutex->unlock(); \
  }
#else
#define DEBUG(arg...)
#endif

#ifdef LOGGER_WARN
#define WARN(arg...) \
  {                  \
    LoggerMutex::logger_mutex->lock(); \
    printf("\033[31m[WARN]"); \
    printf(arg);     \
    printf("\033[0m");  \
    LoggerMutex::logger_mutex->unlock(); \
  }
#else
#define WARN(arg...)
#endif

#define LOGGER_INFO

#ifdef LOGGER_INFO
#define INFO(arg...) \
  {                  \
    LoggerMutex::logger_mutex->lock(); \
    printf("[INFO]"); \
    printf(arg);     \
    LoggerMutex::logger_mutex->unlock(); \
  }
#else
#define INFO(arg...)
#endif

#endif  // LOGGER_H_