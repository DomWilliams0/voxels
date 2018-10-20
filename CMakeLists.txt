cmake_minimum_required(VERSION 3.9)
project(voxel_game C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_executable(${PROJECT_NAME}
        main.c
        src/voxel_game.h
        src/voxel_game.c
        src/voxel_game.c
        src/error.h
        src/log.h
        )

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
