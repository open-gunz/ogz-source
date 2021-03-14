cmake_minimum_required(VERSION 3.16)
project(radio-proxy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package (Threads)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(radio-proxy src/radio-proxy.cpp src/err.cpp src/err.h src/radio-proxy.h src/ICYResponse.cpp src/ICYResponse.h src/DirectResponse.cpp src/DirectResponse.h src/ClientResponse.cpp src/ClientResponse.h src/UDPServer.cpp src/UDPServer.h src/UDPClient.cpp src/UDPClient.h)
target_link_libraries(radio-proxy ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})