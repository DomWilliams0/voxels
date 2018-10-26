#ifndef VOXEL_GAME_CAMERA_H
#define VOXEL_GAME_CAMERA_H

#include "cglm/cglm.h"


struct camera {
    vec3 pos;

    float yaw;
    float pitch;

    mat4 transform;
};

void camera_init(struct camera *camera, vec3 start_pos, vec3 start_dir);

void camera_turn(struct camera *camera, vec2 screen_delta);

#endif
