#ifndef VOXEL_GAME_RENDERER_H
#define VOXEL_GAME_RENDERER_H

#include "error.h"

struct renderer {
    struct world *world;
    int world_vao;
    int world_program;

    int is_wireframe;
};

struct camera;

ERR renderer_init(struct renderer *renderer, int width, int height);

void renderer_toggle_wireframe(struct renderer *renderer);

// TODO renderer_ prefix
void render(struct renderer *renderer, struct camera *camera, float interpolation);

void resize_callback(int width, int height);


#endif
