cmake_minimum_required(VERSION 3.10)
project(webserv)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -std=c++98")


include_directories(
	include
)

set(SOURCES
	webserv.cpp
)

enable_testing()
add_subdirectory(tests)