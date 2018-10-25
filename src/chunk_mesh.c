#include <cglm/vec3.h>
#include "chunk_mesh.h"

// TODO use in face occlusion
static inline void expand_flat_index(uint idx, ivec3 out) {
    int a_shift = CHUNK_WIDTH_SHIFT + CHUNK_HEIGHT_SHIFT;
    int a = 1 << a_shift;
    out[2] = idx >> a_shift;
    uint b = idx - a * out[2];
    out[1] = b >> CHUNK_WIDTH_SHIFT;
    out[0] = b & (CHUNK_WIDTH - 1);
}


static void *shared_mesh_buffer = NULL;
void *chunk_mesh_gen(struct chunk *chunk) {
    struct block block;
    for (uint i = 0; i < BLOCKS_PER_CHUNK; ++i) {
        chunk_get_block_idx(chunk, i, &block);
        // TODO cull if totally occluded

        // cull air blocks
        if (block.type == BLOCK_AIR)
            continue;


        // TODO actually write to buffer
    }

    return shared_mesh_buffer;
}
