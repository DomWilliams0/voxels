#ifndef VOXEL_GAME_CHUNK_MESH_H
#define VOXEL_GAME_CHUNK_MESH_H

#include "world.h"

#define BLOCK_SIZE (0.1f)

// vec3f (pos) + vec4b (colour) + float (ao)
#define CHUNK_MESH_WORDS_PER_INSTANCE (3 + 1 + 1)

// 12 triangles
#define CHUNK_MESH_VERTICES_PER_BLOCK (12 * 3)

struct chunk_mesh_meta {
    int vertex_count;
};

int *chunk_mesh_gen(struct chunk *chunk, struct chunk_mesh_meta *meta);


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

#endif
