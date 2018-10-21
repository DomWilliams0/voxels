cmake_minimum_required(VERSION 3.9)
project(voxel_game C CXX)

set(SRCS
        ../main.c
        voxel_game.h
        voxel_game.c
        voxel_game.c
        error.h
        log.h
        renderer.c renderer.h world.c world.h chunk_mesh.c chunk_mesh.h)

set(LIB ../lib)
file(GLOB LIB_SRCS ${LIB}/gclm/src/* ${LIB}/vec/src/*)

add_executable(${PROJECT_NAME} ${SRCS} ${LIB_SRCS})

include_directories(.)

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
include_directories(${LIB}/cglm/include)

# vec
include_directories(${LIB}/vec/src)

# math
target_link_libraries(${PROJECT_NAME} m)