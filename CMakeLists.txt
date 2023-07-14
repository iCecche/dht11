cmake_minimum_required(VERSION 3.18.4)
project(dht11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lwiringPi -lcurl")

add_executable(dht11 main.cpp dht11.h dht11.cpp)
TARGET_LINK_LIBRARIES(dht11 wiringPi curl)
