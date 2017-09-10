cmake_minimum_required(VERSION 3.9)
project(HTTP-Suite)

set(CMAKE_CXX_STANDARD 14)

set(HTTPC_SOURCE_FILES
        httpc/cmdline.c
        httpc/httpc.cpp)
set(HTTPD_SOURCE_FILES
        httpd/httpd.cpp
        httpd/cmdline.c httpd/HTTPResponderThread.cpp httpd/HTTPResponderThread.h httpd/HTTPHeader.cpp httpd/HTTPHeader.h)

add_executable(httpc ${HTTPC_SOURCE_FILES})
add_executable(httpd ${HTTPD_SOURCE_FILES})
