#ifndef VOXEL_GAME_RENDERER_H
#define VOXEL_GAME_RENDERER_H

#include "chunk_mesh.h"

struct renderer {
    struct world *world;
    int world_vao;
    int world_program;
};

void renderer_init(struct renderer *renderer, int width, int height);

void render(struct renderer *renderer);

void resize_callback(int width, int height);


#endif
