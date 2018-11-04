#ifndef VOXEL_GAME_CAMERA_H
#define VOXEL_GAME_CAMERA_H

#include "cglm/cglm.h"

#define CAMERA_TURN_SPEED (0.001f)
#define CAMERA_MOVE_SPEED (10)

struct camera_state {
    vec3 pos;
    versor rot;

    mat4 transform;

    vec3 forward, right;
};

struct camera {
    struct camera_state state;
    float yaw, pitch;
};

void camera_init(struct camera *camera, vec3 start_pos, vec3 start_dir);

void camera_turn(struct camera *camera, vec2 screen_delta);

void camera_tick(struct camera *camera, double dt);

void camera_interpolate(struct camera_state *last, struct camera_state *new, double alpha, struct camera_state *out);

#endif
