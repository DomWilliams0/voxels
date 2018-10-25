#include "face.h"

inline static void vec_set(ivec3 vec, int x, int y, int z) {
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
}

void face_offset(enum face face, ivec3 out) {
    switch (face) {
        case FACE_FRONT:
            vec_set(out, -1, 0, 0);
            break;
        case FACE_LEFT:
            vec_set(out, 0, 0, -1);
            break;
        case FACE_RIGHT:
            vec_set(out, 0, 0, 1);
            break;
        case FACE_TOP:
            vec_set(out, 0, 1, 0);
            break;
        case FACE_BOTTOM:
            vec_set(out, 0, -1, 0);
            break;
        case FACE_BACK:
            vec_set(out, -1, 0, 0);
            break;
    }
}
