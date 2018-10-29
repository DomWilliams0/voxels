#include <cglm/vec3.h>
#include "chunk_mesh.h"
#include "face.h"

static int shared_mesh_buffer[BLOCKS_PER_CHUNK * CHUNK_MESH_WORDS_PER_INSTANCE * CHUNK_MESH_VERTICES_PER_BLOCK];

static const float BLOCK_VERTICES[];
static const float AO_CURVE[] = {0, 0.6f, 0.8f, 1};

int *chunk_mesh_gen(struct chunk *chunk, struct chunk_mesh_meta *meta) {

    struct block block = {0};
    ivec3 block_pos;
    size_t out_idx = 0;

    for (uint i = 0; i < BLOCKS_PER_CHUNK; ++i) {
        chunk_get_block_idx(chunk, i, &block);
        // cull if totally occluded
        if (block.face_visibility == FACE_VISIBILITY_NONE)
            continue;

        // cull air blocks
        if (block.type == BLOCK_AIR)
            continue;

        expand_flat_index(i, block_pos);

        for (int face_idx = 0; face_idx < FACE_COUNT; ++face_idx) {
            enum face face = FACES[face_idx];

            if (!FACE_IS_VISIBLE(block.face_visibility, face))
                continue;

            int stride = 6 * 6; // 6 vertices * 6 floats per face
            const float *verts = BLOCK_VERTICES + (stride * (int) face);

            union {
                float f;
                int i;
            } f_or_i;

            for (int v = 0; v < 6; ++v) {
                // vertex pos in chunk space
                int v_idx = v * 6;
                for (int j = 0; j < 3; ++j) {
                    f_or_i.f = verts[v_idx + j] + block_pos[j] * 2 * BLOCK_SIZE;
                    shared_mesh_buffer[out_idx++] = f_or_i.i;
                }

                // colour
                int colour = block_type_colour(block.type);
                shared_mesh_buffer[out_idx++] = colour;

                // ao
                char ao_idx = ao_get_vertex(block.ao, face, v);
                float ao = AO_CURVE[ao_idx];
                f_or_i.f = ao;
                shared_mesh_buffer[out_idx++] = f_or_i.i;
            }
        }
    }

    meta->vertex_count = (int) out_idx;
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
