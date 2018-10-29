#include "face.h"

const enum face FACES[] = {
        FACE_FRONT,
        FACE_LEFT,
        FACE_RIGHT,
        FACE_TOP,
        FACE_BOTTOM,
        FACE_BACK
};


inline static void vec_add(const ivec3 in, ivec3 out, int x, int y, int z) {
    out[0] = in[0] + x;
    out[1] = in[1] + y;
    out[2] = in[2] + z;
}

void face_offset(enum face face, const ivec3 in, ivec3 out) {
    switch (face) {
        case FACE_FRONT:
            vec_add(in, out, -1, 0, 0);
            break;
        case FACE_LEFT:
            vec_add(in, out, 0, 0, -1);
            break;
        case FACE_RIGHT:
            vec_add(in, out, 0, 0, 1);
            break;
        case FACE_TOP:
            vec_add(in, out, 0, 1, 0);
            break;
        case FACE_BOTTOM:
            vec_add(in, out, 0, -1, 0);
            break;
        case FACE_BACK:
            vec_add(in, out, 1, 0, 0);
            break;
    }
}

enum face face_opposite(enum face face) {
    switch (face) {
        case FACE_FRONT:
            return FACE_BACK;
        case FACE_LEFT:
            return FACE_RIGHT;
        case FACE_RIGHT:
            return FACE_LEFT;
        case FACE_TOP:
            return FACE_BOTTOM;
        case FACE_BOTTOM:
            return FACE_TOP;
        case FACE_BACK:
            return FACE_FRONT;
    }
}
