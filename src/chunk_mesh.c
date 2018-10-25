#include <cglm/vec3.h>
#include "chunk_mesh.h"
#include "face.h"

static inline void expand_flat_index(uint idx, ivec3 out) {
    int a_shift = CHUNK_WIDTH_SHIFT + CHUNK_HEIGHT_SHIFT;
    int a = 1 << a_shift;
    out[2] = idx >> a_shift;
    uint b = idx - a * out[2];
    out[1] = b >> CHUNK_WIDTH_SHIFT;
    out[0] = b & (CHUNK_WIDTH - 1);
}


static int shared_mesh_buffer[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH *
                              CHUNK_MESH_WORDS_PER_INSTANCE * CHUNK_MESH_VERTICES_PER_BLOCK];

static const float BLOCK_VERTICES[];

int *chunk_mesh_gen(struct chunk *chunk) {

    struct block block = {0};
    ivec3 block_pos;
    int out_idx = 0;

    for (uint i = 0; i < BLOCKS_PER_CHUNK; ++i) {
        chunk_get_block_idx(chunk, i, &block);
        // TODO cull if totally occluded

        // cull air blocks
        if (block.type == BLOCK_AIR)
            continue;

        expand_flat_index(i, block_pos);

        for (int face_idx = 0; face_idx < FACE_COUNT; ++face_idx) {
            enum face face = FACES[face_idx];

            // TODO check face is visible
            int stride = 6 * 6; // 6 vertices * 6 floats per face
            const float *verts = BLOCK_VERTICES + (stride * (int) face);

            for (int v = 0; v < 6; ++v) {
                // vertex pos in chunk space
                int v_idx = v * 6;
                for (int j = 0; j < 3; ++j) {
                    union {
                        float f;
                        int i;
                    } f_or_i;

                    f_or_i.f = verts[v_idx + j] + block_pos[j] * 2 * BLOCK_SIZE;
                    shared_mesh_buffer[out_idx++] = f_or_i.i;
                }

                // TODO colour
                int colour = block_type_colour(block.type);
                shared_mesh_buffer[out_idx++] = colour;

                // TODO ao
                int ao = 1;
                shared_mesh_buffer[out_idx++] = ao;
            }
        }
    }

    // TODO left over junk in shared buffer? should store how much is used by this chunk

    return shared_mesh_buffer;
}

// for brevity
#define X BLOCK_SIZE

static const float BLOCK_VERTICES[] = {
        // front
        -X, -X, -X,  // vertex pos
        -1, +0, +0,  // face normal
        -X, -X, +X,
        -1, +0, +0,
        -X, +X, +X,
        -1, +0, +0,
        -X, +X, +X,
        -1, +0, +0,
        -X, +X, -X,
        -1, +0, +0,
        -X, -X, -X,
        -1, +0, +0,

        // left
        +X, -X, -X,
        +0, +0, -1,
        -X, -X, -X,
        +0, +0, -1,
        -X, +X, -X,
        +0, +0, -1,
        -X, +X, -X,
        +0, +0, -1,
        +X, +X, -X,
        +0, +0, -1,
        +X, -X, -X,
        +0, +0, -1,

        // right
        -X, -X, +X,
        +0, +0, +1,
        +X, -X, +X,
        +0, +0, +1,
        +X, +X, +X,
        +0, +0, +1,
        +X, +X, +X,
        +0, +0, +1,
        -X, +X, +X,
        +0, +0, +1,
        -X, -X, +X,
        +0, +0, +1,

        // top
        -X, +X, -X,
        +0, +1, +0,
        -X, +X, +X,
        +0, +1, +0,
        +X, +X, +X,
        +0, +1, +0,
        +X, +X, +X,
        +0, +1, +0,
        +X, +X, -X,
        +0, +1, +0,
        -X, +X, -X,
        +0, +1, +0,

        // bottom
        +X, -X, -X,
        +0, -1, +0,
        +X, -X, +X,
        +0, -1, +0,
        -X, -X, +X,
        +0, -1, +0,
        -X, -X, +X,
        +0, -1, +0,
        -X, -X, -X,
        +0, -1, +0,
        +X, -X, -X,
        +0, -1, +0,

        // back
        +X, +X, -X,
        +1, +0, +0,
        +X, +X, +X,
        +1, +0, +0,
        +X, -X, +X,
        +1, +0, +0,
        +X, -X, +X,
        +1, +0, +0,
        +X, -X, -X,
        +1, +0, +0,
        +X, +X, -X,
        +1, +0, +0
};
#undef X
