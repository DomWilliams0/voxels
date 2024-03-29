#ifndef VOXEL_GAME_WORLD_H
#define VOXEL_GAME_WORLD_H

#include <cglm/cglm.h>
#include "phys.hpp"
#include "error.h"
#include "face.h"

#define CHUNK_WIDTH_SHIFT 5
#define CHUNK_HEIGHT_SHIFT 5

#define CHUNK_WIDTH (1<<CHUNK_WIDTH_SHIFT)
#define CHUNK_DEPTH (CHUNK_WIDTH)
#define CHUNK_HEIGHT (1<<CHUNK_HEIGHT_SHIFT)

#define BLOCKS_PER_CHUNK (CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT)

#define MAX_DYN_VOXELS 800

struct world;
struct chunk;

ERR world_load_demo(struct world **world, const char *name);

void world_destroy(struct world *world);

void world_tick_physics(struct world *world, double dt);

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

struct chunk_render_objs {
    int vao, vbo;
};

int chunk_has_flag(struct chunk *chunk, enum chunk_flag flag);

struct chunk_render_objs *chunk_render_objs(struct chunk *chunk);

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

// 2 bits per 4 unique vertices per 6 faces
#define AO_BIT_COUNT (2*4*6)

struct block {
    enum block_type type;

    int face_visibility : 6;
    long ao: AO_BIT_COUNT;
};


// TODO return ptr to avoid copy
void chunk_get_block_idx(struct chunk *chunk, int idx, struct block *out);

int chunk_is_solid_at(struct chunk *chunk, ivec3 block_pos);

void chunk_get_pos(struct chunk *chunk, ivec3 out);

// helper
void expand_flat_index(uint idx, ivec3 out);

void chunk_init_lighting(struct world *world, struct chunk *chunk);

void chunk_update_collision_mesh(struct world *world, struct chunk *chunk);

// all bits set = AO_VERTEX_NONE for each
#define AO_BLOCK_NONE ((1L << AO_BIT_COUNT)-1)
#define AO_VERTEX_NONE (3)
#define AO_VERTEX_FULL (0)

// 1 byte = 4 vertices * 2 bits each
// or with accumulator
long ao_set_face(enum face face, char v05, char v1, char v23, char v4);

char ao_get_vertex(long ao, enum face face, int vertex_idx);


// modification
void world_set_block(struct world *world, ivec3 pos, enum block_type type);

// dynamic voxels
struct dyn_voxel {
    enum block_type type;
    struct phys_dyn_voxel phys_body;
};

struct dyn_voxel_iterator {
    struct dyn_voxel *current;
    int valid;
    int _progress;
};

#define DYN_VOXEL_ITER_INIT {._progress=-1, .valid=0}

int world_dyn_voxels_next(struct world *world, struct dyn_voxel_iterator *it);


#endif
