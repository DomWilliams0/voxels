cmake_minimum_required(VERSION 3.9)
project(voxel_lib C CXX)

set(SRCS voxel_game.h voxel_game.c error.h log.h renderer.c renderer.h world.c world.h chunk_mesh.c chunk_mesh.h face.h face.c camera.c camera.h collision_mesh.c collision_mesh.h)

# resources
#file(COPY ../res DESTINATION ${CMAKE_BINARY_DIR})
ADD_CUSTOM_TARGET(link_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../res ${CMAKE_BINARY_DIR}/res)


file(GLOB LIB_SRCS ${LIB}/gclm/src/* ${LIB}/vec/src/*)
add_library(${PROJECT_NAME} ${SRCS} ${LIB_SRCS})
target_compile_options(${PROJECT_NAME} PRIVATE -O1)

include_directories(.)

# sdl
find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
endif ()

# glew
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
endif()

# opengl
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif ()

# cglm and vec
include_directories(${LIB_INCS})

# math
target_link_libraries(${PROJECT_NAME} m)