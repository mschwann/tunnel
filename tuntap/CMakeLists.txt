cmake_minimum_required(VERSION 3.1...3.27)

add_library (tuntap
    tuntap.cpp
)
target_compile_options(tuntap PRIVATE -Wall -Wextra -Wpedantic -Werror)

add_executable(tun_run main.cpp)
target_link_libraries(tun_run tuntap)
