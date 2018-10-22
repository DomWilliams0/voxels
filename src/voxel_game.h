#ifndef VOXEL_GAME_GAME_H
#define VOXEL_GAME_GAME_H

#include <GLFW/glfw3.h>
#include "renderer.h"
#include "error.h"

struct voxel_game {
    GLFWwindow *window;
    struct renderer renderer;
};

ERR game_init(struct voxel_game *game, int width, int height);

void game_start(struct voxel_game *game);

void game_destroy(struct voxel_game *game);

#endif
