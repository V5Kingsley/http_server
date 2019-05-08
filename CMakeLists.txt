project(http_server)

add_compile_options(-std=c++11)

find_package(Threads REQUIRED)

include_directories(base logger timer)

add_library(parameters parameters.cpp)
add_library(my_thread base/my_thread.cpp)
add_library(my_condition base/my_condition.cpp)
add_library(work_thread work_thread.cpp)
add_library(thread_pool thread_pool.cpp)
add_library(logger logger/logger.cpp)
add_library(Socket Socket.cpp)
add_library(TcpServer TcpServer.cpp)

add_library(TcpEpollServer TcpEpollServer.cpp)
target_link_libraries(TcpEpollServer TcpServer)

add_executable(httpserver main.cpp)
target_link_libraries(httpserver TcpEpollServer Socket logger parameters thread_pool my_thread my_condition work_thread ${CMAKE_THREAD_LIBS_INIT})


#add_executable(pool_test test/pool_test.cpp)
#target_link_libraries(pool_test logger parameters thread_pool my_thread my_condition work_thread ${CMAKE_THREAD_LIBS_INIT})

#add_executable(condition_test test/condition_test.cpp)
#target_link_libraries(condition_test logger parameters thread_pool my_thread my_condition work_thread ${CMAKE_THREAD_LIBS_INIT})

#add_executable(timer_test test/timer_test.cpp)

