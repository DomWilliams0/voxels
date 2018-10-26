#ifndef VOXEL_GAME_CAMERA_H
#define VOXEL_GAME_CAMERA_H

#include "cglm/cglm.h"

#define CAMERA_TURN_SPEED (0.01f)
#define CAMERA_MOVE_SPEED (0.03f)

struct camera {
    vec3 pos;
    float yaw, pitch;

    mat4 transform;

    vec3 forward, right;
};

void camera_init(struct camera *camera, vec3 start_pos, vec3 start_dir);

void camera_turn(struct camera *camera, vec2 screen_delta);

void camera_tick(struct camera *camera);

#endif
