#ifndef VOXEL_GAME_RENDERER_H
#define VOXEL_GAME_RENDERER_H

#include "error.h"

struct renderer {
    struct world *world;
    int world_program;

    int dyn_program;
    int dyn_mesh_vbo;
    int dyn_vao, dyn_vbo;

    int is_wireframe;
};

struct camera;

ERR renderer_init(struct renderer *renderer, int width, int height);

void renderer_toggle_wireframe(struct renderer *renderer);

struct camera_state;
void renderer_render(struct renderer *renderer, struct camera_state *camera_state, float interpolation);

void resize_callback(int width, int height);


#endif
