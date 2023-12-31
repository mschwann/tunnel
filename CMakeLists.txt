cmake_minimum_required(VERSION 3.1...3.27)
project (tunnel)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set (CMAKE_CXX_STANDARD 20)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)

FIND_PACKAGE(Boost 1.74.0 REQUIRED COMPONENTS system thread regex)

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

INCLUDE_DIRECTORIES(
    libs/spdlog/include
    build/include
    socket/include
    tuntap/include
    config/include
)
LINK_DIRECTORIES(
    build/libviface/src
    build/socket
)

add_subdirectory (libs/spdlog)

add_subdirectory (tuntap)
add_subdirectory (socket)
add_subdirectory (server)
add_subdirectory (client)