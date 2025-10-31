cmake_minimum_required(VERSION 3.20)
project(dircmp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dircmp src/main.cpp)
