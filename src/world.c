#include <cglm/cglm.h>
#include <vec.h>
// TODO consistent style in including libs, <> or ""
#include "world.h"
#include "log.h"
#include "chunk_mesh.h"

#define BLOCKS_PER_CHUNK CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT

enum block_type {
    BLOCK_AIR = 0,
    BLOCK_GROUND = 1,
    BLOCK_OBJECT = 2,
};

struct block {
    enum block_type type;
    // TODO face visibility
    // TODO ambient occlusion
};

struct chunk {
    ivec3 pos;
    chunk_vbo vbo;

    struct block blocks[BLOCKS_PER_CHUNK];

};

typedef vec_t(struct chunk *) chunk_vec_t;

struct world {
    chunk_vec_t chunks;
};

// helper
static inline void glm_ivec_copy(const ivec3 src, ivec3 dst) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

static inline void glm_ivec_zero(ivec3 vec) {
    vec[0] = 0;
    vec[1] = 0;
    vec[2] = 0;
}

#define IVEC3_COPY(vec) {(vec)[0], (vec)[1], (vec)[2]}

// TODO chunk dealloc
static struct chunk *chunk_alloc() {
    struct chunk *chunk = malloc(sizeof(struct chunk));
    if (!chunk) {
        LOG_ERROR("failed to alloc chunk");
    } else {
        chunk->vbo = 0;
        glm_ivec_zero(chunk->pos);
    }
    return chunk;
}

static void world_init(struct world *world) {
    vec_init(&world->chunks);
}

static void world_add_chunk(struct world *world, ivec3 pos) {
    struct chunk *c = chunk_alloc();
    glm_ivec_copy(pos, c->pos);
    vec_push(&world->chunks, c);
}

// mods each coord to chunk range
static void world_to_chunk_coords(ivec3 pos) {
    pos[0] &= (CHUNK_WIDTH - 1); // modulus as Chunk dimensions are a multiple of 2
    pos[1] &= (CHUNK_HEIGHT - 1);
    pos[2] &= (CHUNK_WIDTH - 1);
}

// divs each coord to get the owning chunk coords
static void resolve_chunk_coords(ivec3 pos) {
    pos[0] >>= CHUNK_WIDTH_SHIFT;
    pos[1] >>= CHUNK_HEIGHT_SHIFT;
    pos[2] >>= CHUNK_WIDTH_SHIFT;
}

static struct chunk *world_find_chunk(struct world *world, ivec3 block_pos) {
    ivec3 chunk_pos = IVEC3_COPY(block_pos);
    resolve_chunk_coords(chunk_pos);

    int i;
    struct chunk *c;
    vec_foreach(&world->chunks, c, i) {
            if (c->pos[0] == chunk_pos[0] &&
                c->pos[1] == chunk_pos[1] &&
                c->pos[2] == chunk_pos[2])
                return c;
        }

    return NULL;
}

static struct block *chunk_get_block(struct chunk *chunk, const ivec3 pos) {
    int idx = (CHUNK_WIDTH * CHUNK_HEIGHT * pos[2]) + (CHUNK_WIDTH * pos[1]) + pos[0];
    if (idx < 0 || idx >= BLOCKS_PER_CHUNK)
        return NULL;

    return &chunk->blocks[idx];
}

static struct block *world_get_block(struct world *world, ivec3 pos) {
    struct chunk *chunk = world_find_chunk(world, pos);
    if (!chunk) return NULL;

    ivec3 chunk_local_pos = IVEC3_COPY(pos);
    world_to_chunk_coords(chunk_local_pos);

    return chunk_get_block(chunk, chunk_local_pos);
}

// helper
static void world_set_block(struct world *world, ivec3 pos, enum block_type type) {
    struct block *b = world_get_block(world, pos);
    if (b) {
        b->type = type;
        // TODO mark as dirty
    }
}

ERR world_load_demo(struct world **world, const char *name) {
    struct world *w = malloc(sizeof(struct world));
    if (!w) {
        LOG_ERROR("failed to allocate world");
        return ERR_FAIL;
    }

    world_init(w);

    world_add_chunk(w, (ivec3) {0, 0, 0});
    world_set_block(w, (ivec3) {1, 1, 1}, BLOCK_GROUND);

    // TODO actually use name

    *world = w;
    return ERR_SUCC;
}

void world_destroy(struct world *world) {
    if (world) {
        // TODO use chunk_dealloc function
        struct chunk *c;
        int i;
        vec_foreach(&world->chunks, c, i) {
                free(c);
            }
        vec_deinit(&world->chunks);
    }
}
