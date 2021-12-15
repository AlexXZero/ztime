cmake_minimum_required(VERSION 3.20.0)

# Add the board
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ztime)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE src)
