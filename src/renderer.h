#ifndef VOXEL_GAME_RENDERER_H
#define VOXEL_GAME_RENDERER_H

#include <GLFW/glfw3.h>

void renderer_init(int width, int height);

void render();

void resize_callback(GLFWwindow *window, int width, int height);


#endif
