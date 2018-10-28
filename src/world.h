#ifndef VOXEL_GAME_WORLD_H
#define VOXEL_GAME_WORLD_H

#include "error.h"
#include <cglm/cglm.h>

#define CHUNK_WIDTH_SHIFT 5
#define CHUNK_HEIGHT_SHIFT 5

#define CHUNK_WIDTH (1<<CHUNK_WIDTH_SHIFT)
#define CHUNK_DEPTH (CHUNK_WIDTH)
#define CHUNK_HEIGHT (1<<CHUNK_HEIGHT_SHIFT)

#define BLOCKS_PER_CHUNK (CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT)


struct world;
struct chunk;

ERR world_load_demo(struct world **world, const char *name);

void world_destroy(struct world *world);

enum chunk_flag {
    CHUNK_FLAG_NEW = 1 << 0,
    CHUNK_FLAG_VISIBLE = 1 << 1,
    CHUNK_FLAG_DIRTY = 1 << 2
};


// `current` will be NULL if finished/invalid
struct chunk_iterator {
    struct chunk *current;
    int _progress;
};

int chunk_has_flag(struct chunk *chunk, enum chunk_flag flag);

int *chunk_vbo(struct chunk *chunk);

struct chunk_mesh_meta *chunk_mesh_meta(struct chunk *chunk);

void chunk_world_space_pos(struct chunk *chunk, vec3 out);

void world_chunks_first(struct world *world, struct chunk_iterator *it);

void world_chunks_next(struct world *world, struct chunk_iterator *it);

void world_chunks_clear_dirty(struct world *world);

enum block_type {
    BLOCK_AIR = 0,
    BLOCK_GROUND = 1,
    BLOCK_OBJECT = 2,
};

// rgba reversed so abgr
int block_type_colour(enum block_type type);

int block_type_opaque(enum block_type type);

struct block {
    enum block_type type;
    int face_visibility;
    // TODO ambient occlusion
};


void chunk_get_block_idx(struct chunk *chunk, int idx, struct block *out);

void chunk_get_pos(struct chunk *chunk, ivec3 out);

// helper
void expand_flat_index(uint idx, ivec3 out);

void chunk_init_lighting(struct world *world, struct chunk *chunk);

#endif
