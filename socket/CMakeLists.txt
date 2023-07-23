cmake_minimum_required(VERSION 3.1...3.27)

add_library (socket 
    Socket.cpp
)

target_link_libraries(socket ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})
