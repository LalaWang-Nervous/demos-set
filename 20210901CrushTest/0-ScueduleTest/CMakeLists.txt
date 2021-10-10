cmake_minimum_required(VERSION 3.20)
project(crush)

set(CMAKE_CXX_STANDARD 14)

add_executable(crush main.cpp WeightedHashMap.cpp WeightedHashMap.h)
