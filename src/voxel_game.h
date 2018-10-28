#ifndef VOXEL_GAME_GAME_H
#define VOXEL_GAME_GAME_H

#include <SDL2/SDL.h>
#include "renderer.h"
#include "error.h"

struct voxel_game {
    SDL_Window *window;
    struct renderer renderer;
};

ERR game_init(struct voxel_game *game, int width, int height);

void game_start(struct voxel_game *game, int argc, char **argv);

void game_destroy(struct voxel_game *game);

#endif
