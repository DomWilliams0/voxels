cmake_minimum_required(VERSION 3.9)
project(voxel_game C CXX)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(SRCS
        main.c
        src/voxel_game.h
        src/voxel_game.c
        src/voxel_game.c
        src/error.h
        src/log.h
        src/renderer.c src/renderer.h)

file(GLOB LIB_SRCS lib/gclm/src/* lib/vec/src/*)

add_executable(${PROJECT_NAME} ${SRCS} ${LIB_SRCS})

include_directories(src)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
find_package(GLFW REQUIRED)
if (GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})
endif ()

# opengl
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    include_directories(${PROJECT_NAME} ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif ()

# cglm
include_directories(lib/cglm/include)

# vec
include_directories(lib/vec/src)

# math
target_link_libraries(${PROJECT_NAME} m)