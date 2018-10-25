#ifndef VOXEL_GAME_CHUNK_MESH_H
#define VOXEL_GAME_CHUNK_MESH_H

#include "world.h"

#define BLOCK_SIZE (0.1f)

// vec3f (pos) + vec4b (colour) + float (ao)
#define CHUNK_MESH_WORDS_PER_INSTANCE (3 + 1 + 1)

// 12 triangles
#define CHUNK_MESH_VERTICES_PER_BLOCK (12 * 3)

int * chunk_mesh_gen(struct chunk *chunk);

#endif
