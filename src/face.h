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

extern const enum face FACES[];

#define FACE_COUNT 6

void face_offset(enum face face, const ivec3 in, ivec3 out);

enum face face_opposite(enum face face);

#define FACE_VISIBILITY_ALL ((1 << FACE_COUNT)-1)
#define FACE_VISIBILITY_NONE (0)
#define FACE_IS_VISIBLE(vis, face) (((vis) & (1<<(face))) != 0)

#endif
