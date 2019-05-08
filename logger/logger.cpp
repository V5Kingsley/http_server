/**
 * @file logger.cpp
 * @author Kingsley
 * @brief 
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "logger.h"

LoggerMutex* LoggerMutex::logger_mutex = new LoggerMutex();