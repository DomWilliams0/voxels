cmake_minimum_required(VERSION 3.9)
project(voxel_game C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

get_filename_component(LIB lib ABSOLUTE)
file(GLOB LIB_SRCS ${LIB}/gclm/src/* ${LIB}/vec/src/*)
set(LIB_INCS ${LIB}/cglm/include ${LIB}/vec/src)

add_executable(${PROJECT_NAME} main.c)
include_directories(src)
include_directories(phys)

add_subdirectory(src)
add_subdirectory(phys)

enable_testing()
add_subdirectory(tests)

target_link_libraries(${PROJECT_NAME} voxel_lib voxel_phys)