cmake_minimum_required(VERSION 3.5)
project(skeleton)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    debugfs.c
    debugfs.h
    Makefile
    util.c
    util.h
    vfat.c
    vfat.h)

add_executable(skeleton ${SOURCE_FILES})