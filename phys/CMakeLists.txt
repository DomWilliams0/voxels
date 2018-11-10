cmake_minimum_required(VERSION 3.9)
project(voxel_phys CXX)

set(SRCS phys.hpp phys.cpp)

include_directories(.)

add_library(${PROJECT_NAME} SHARED ${SRCS})

# reactphysics3d
add_subdirectory(lib/reactphysics3d)
target_link_libraries(${PROJECT_NAME} reactphysics3d)
include_directories(lib/reactphysics3d/src)
