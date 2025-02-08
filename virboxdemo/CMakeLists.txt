cmake_minimum_required(VERSION 3.10)
set(TARGET_NAME virboxdemo)
project(${TARGET_NAME})

# set(TARGET_NAME helloworld)
set(CMAKE_SYSTEM_NAME "Android")

add_executable(${TARGET_NAME} main.cpp)

find_library(log-lib log)
target_link_libraries(${TARGET_NAME} PRIVATE ${log-lib})