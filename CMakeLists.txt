cmake_minimum_required(VERSION 3.1...3.27)
project (tunnel)

set (CMAKE_CXX_STANDARD 20)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)

FIND_PACKAGE(Boost 1.74.0 REQUIRED COMPONENTS system thread regex)

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

INCLUDE_DIRECTORIES(
    libs/libviface/include/
    libs/spdlog/include
    build/include
    socket/include
    tuntap/include
)
LINK_DIRECTORIES(
    build/libviface/src
    build/socket
)

#submodule - libs/libviface
set(libviface_VERSION_STRING "1.1.0")
add_subdirectory(libs/libviface/src)
add_subdirectory (libs/spdlog)

add_subdirectory (tuntap)
add_subdirectory (socket)
add_subdirectory (server)
add_subdirectory (client)