cmake_minimum_required(VERSION 3.9)
project(HTTP-Suite CXX)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)

set(HTTPC_SOURCE_FILES
        httpc/cmdline.c
        httpc/httpc.cpp)
set(HTTPD_SOURCE_FILES
        httpd/httpd.cpp
        httpd/cmdline.c
        httpd/cmdline.h
        httpd/HTTPResponder.cpp
        httpd/HTTPResponder.h
        httpd/HTTPHeader.cpp
        httpd/HTTPHeader.h)

add_executable(httpc ${HTTPC_SOURCE_FILES})
add_executable(httpd ${HTTPD_SOURCE_FILES})
target_link_libraries(httpd ${CMAKE_THREAD_LIBS_INIT})
