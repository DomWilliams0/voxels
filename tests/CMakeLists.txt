cmake_minimum_required(VERSION 3.9)
project(voxel_tests C CXX)

add_executable(${PROJECT_NAME} tests.c)

find_package(Check REQUIRED)
if (CHECK_FOUND)
    include_directories(${PROJECT_NAME} ${CHECK_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CHECK_LIBRARIES})
endif ()

include_directories(${PROJECT_NAME} ../src)
include_directories(${LIB_INCS})
target_link_libraries(${PROJECT_NAME} voxel_lib)

add_test(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

