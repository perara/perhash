cmake_minimum_required(VERSION 3.20)
project(perhash)

set(CMAKE_CXX_STANDARD 14)


add_executable(perhash main.cpp PerHash.h)
target_include_directories(perhash PUBLIC ./)