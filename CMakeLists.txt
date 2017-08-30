cmake_minimum_required(VERSION 3.9)
project(Project_1___HTTPd)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        cmdline.c
        cmdline.h
        httpc.cpp
        Makefile
        options.ggo)

add_executable(Project_1___HTTPd ${SOURCE_FILES})