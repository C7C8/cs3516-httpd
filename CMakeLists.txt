cmake_minimum_required(VERSION 3.9)
project(HTTP-Suite)

set(CMAKE_CXX_STANDARD 11)

set(HTTPC_SOURCE_FILES
        httpc/cmdline.c
        httpc/cmdline.h
        httpc/httpc.cpp)
set(HTTPD_SOURCE_FILES
        httpd/httpd.cpp)

add_executable(httpc ${HTTPC_SOURCE_FILES})
add_executable(httpd ${HTTPD_SOURCE_FILES})
