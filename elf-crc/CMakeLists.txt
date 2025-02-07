cmake_minimum_required(VERSION 3.10)
project(unix_server)

set(TARGET_NAME elf-crc)

find_package(ZLIB REQUIRED)
add_executable(${TARGET_NAME} main.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE ZLIB::ZLIB)