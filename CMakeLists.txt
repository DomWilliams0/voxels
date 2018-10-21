cmake_minimum_required(VERSION 3.9)
project(voxel_game C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} voxel_lib)
include_directories(${PROJECT_NAME} src)

add_subdirectory(src)
enable_testing()
add_subdirectory(tests)