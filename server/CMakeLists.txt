cmake_minimum_required(VERSION 3.1...3.27)


add_executable(server main.cpp)

target_link_libraries(server socket spdlog)
