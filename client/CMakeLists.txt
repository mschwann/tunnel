cmake_minimum_required(VERSION 3.1...3.27)

add_executable(client main.cpp)

target_link_libraries(client socket spdlog)
