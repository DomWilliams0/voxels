#ifndef VOXEL_GAME_FACE_H
#define VOXEL_GAME_FACE_H


#include <cglm/cglm.h>

// TODo store bitwise value outside
enum face {
    FACE_FRONT = 0,
    FACE_LEFT,
    FACE_RIGHT,
    FACE_TOP,
    FACE_BOTTOM,
    FACE_BACK,
};

const enum face FACES[] = {
        FACE_FRONT,
        FACE_LEFT,
        FACE_RIGHT,
        FACE_TOP,
        FACE_BOTTOM,
        FACE_BACK
};

#define FACE_COUNT 6

void face_offset(enum face face, ivec3 out);


#endif
