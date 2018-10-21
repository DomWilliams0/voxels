cmake_minimum_required(VERSION 3.9)
project(voxel_game C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_subdirectory(src)
