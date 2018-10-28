#include <cglm/cglm.h>
#include <vec.h>
// TODO consistent style in including libs, <> or ""
#include "world.h"
#include "log.h"
#include "chunk_mesh.h"
#include "face.h"

struct chunk {
    ivec3 pos;
    int vbo;
    int flags;
    struct chunk_mesh_meta mesh;

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

// helper
static inline int glm_ivec_eq(const ivec3 a, const ivec3 b) {
    return a[0] == b[0] &&
           a[1] == b[1] &&
           a[2] == b[2];
}

// helper
static inline void glm_ivec_zero(ivec3 vec) {
    vec[0] = 0;
    vec[1] = 0;
    vec[2] = 0;
}

#define IVEC3_COPY(vec) {(vec)[0], (vec)[1], (vec)[2]}

// TODO chunk dealloc
static struct chunk *chunk_alloc() {
    struct chunk *chunk = calloc(1, sizeof(struct chunk));
    if (!chunk) {
        LOG_ERROR("failed to alloc chunk");
    } else {
        chunk->vbo = 0;
        chunk->flags = CHUNK_FLAG_NEW | CHUNK_FLAG_DIRTY | CHUNK_FLAG_VISIBLE;
        glm_ivec_zero(chunk->pos);

        struct block default_block = {
                .type = BLOCK_AIR,
                .face_visibility = FACE_VISIBILITY_ALL
        };
        for (int i = 0; i < BLOCKS_PER_CHUNK; ++i) {
            chunk->blocks[i] = default_block;
        }
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

static void chunk_to_world_coords(struct chunk *chunk, ivec3 pos) {
    pos[0] += chunk->pos[0] << CHUNK_WIDTH_SHIFT;
    pos[1] += chunk->pos[1] << CHUNK_HEIGHT_SHIFT;
    pos[2] += chunk->pos[2] << CHUNK_WIDTH_SHIFT;
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

// hint can be NULL
static struct block *world_get_block_with_chunk_hint(struct world *world, struct chunk *hint, ivec3 pos) {
    if (hint) {
        ivec3 chunk_pos = IVEC3_COPY(pos);
        resolve_chunk_coords(chunk_pos);

        if (glm_ivec_eq(chunk_pos, hint->pos)) {
            ivec3 chunk_local_pos = IVEC3_COPY(pos);
            world_to_chunk_coords(chunk_local_pos);

            return chunk_get_block(hint, chunk_local_pos);
        }
    }

    return world_get_block(world, pos);
}

static void world_set_block(struct world *world, ivec3 pos, enum block_type type) {
    struct block *b = world_get_block(world, pos);
    if (b) {
        b->type = type;
        // TODO mark as dirty
    }
}

static void demo_set_block_safely(struct world *world, ivec3 pos, enum block_type type) {
    if (!world_find_chunk(world, pos)) {
        ivec3 chunk_coord = IVEC3_COPY(pos);
        resolve_chunk_coords(chunk_coord);
        world_add_chunk(world, chunk_coord);
    }

    world_set_block(world, pos, type);
}

static void demo_fill_range(struct world *world, const ivec3 start_pos, const ivec3 len, enum block_type type) {
    for (int z = 0; z < len[2]; ++z) {
        for (int y = 0; y < len[1]; ++y) {
            for (int x = 0; x < len[0]; ++x) {
                // TODO use chunk hints?
                demo_set_block_safely(world, (ivec3) {start_pos[0] + x, start_pos[1] + y, start_pos[2] + z}, type);
            }
        }
    }
}

ERR world_load_demo(struct world **world, const char *name) {
    struct world *w = malloc(sizeof(struct world));
    if (!w) {
        LOG_ERROR("failed to allocate world");
        return ERR_FAIL;
    }

    int result = ERR_SUCC;

    world_init(w);

    if (!strcmp(name, "demo")) {
        demo_set_block_safely(w, (ivec3) {0, 0, 0}, BLOCK_GROUND);
        demo_set_block_safely(w, (ivec3) {1, 0, 0}, BLOCK_GROUND);
        demo_set_block_safely(w, (ivec3) {0, 1, 1}, BLOCK_OBJECT);
        demo_fill_range(w, (ivec3) {50, 0, 0}, (ivec3) {1, 20, 20}, BLOCK_OBJECT);
        demo_fill_range(w, (ivec3) {0, 0, 40}, (ivec3) {30, 10, 1}, BLOCK_GROUND);
    } else if (!strcmp(name, "yuge")) {
        demo_fill_range(w, (ivec3) {0, 0, 0}, (ivec3) {60, 60, 60}, BLOCK_OBJECT);
    } else if (!strcmp(name, "building")) {
        int ground = 2;
        demo_fill_range(w, (ivec3) {0, 0, 0}, (ivec3) {100, ground, 100}, BLOCK_GROUND);

        int size = 40;
        int height = 60;
        int thickness = 4;
        int pad = 5;

        // 4 walls
        demo_fill_range(w, (ivec3) {pad, ground, pad}, (ivec3) {size + thickness, height, thickness}, BLOCK_OBJECT);
        demo_fill_range(w, (ivec3) {pad + size, ground, pad}, (ivec3) {thickness, height, size + thickness},
                        BLOCK_OBJECT);
        demo_fill_range(w, (ivec3) {pad, ground, pad + size}, (ivec3) {size, height, thickness}, BLOCK_OBJECT);
        demo_fill_range(w, (ivec3) {pad, ground, pad}, (ivec3) {thickness, height, size}, BLOCK_OBJECT);

        // ceiling
        demo_fill_range(w, (ivec3) {pad, height + 1, pad}, (ivec3) {size, 1, size}, BLOCK_OBJECT);

        // door
        int doorWidth = 9;
        int doorHeight = 20;
        demo_fill_range(w, (ivec3) {pad + size / 2, ground, pad + size}, (ivec3) {doorWidth, doorHeight, thickness},
                        BLOCK_AIR);
    } else {
        result = ERR_FAIL;
    }


    *world = w;
    return result;
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

int chunk_has_flag(struct chunk *chunk, enum chunk_flag flag) {
    return (chunk->flags & flag) == flag;
}

int *chunk_vbo(struct chunk *chunk) {
    return &chunk->vbo;
}

struct chunk_mesh_meta *chunk_mesh_meta(struct chunk *chunk) {
    return &chunk->mesh;
}

void chunk_world_space_pos(struct chunk *chunk, vec3 out) {
    out[0] = chunk->pos[0] * CHUNK_WIDTH * BLOCK_SIZE * 2;
    out[1] = chunk->pos[1] * CHUNK_HEIGHT * BLOCK_SIZE * 2;
    out[2] = chunk->pos[2] * CHUNK_DEPTH * BLOCK_SIZE * 2;
}

void world_chunks_first(struct world *world, struct chunk_iterator *it) {
    if (world->chunks.length > 0) {
        it->_progress = 1;
        it->current = vec_first(&world->chunks);
    } else {
        it->current = NULL;
    }
}

void world_chunks_next(struct world *world, struct chunk_iterator *it) {
    if (it->_progress < world->chunks.length) {
        it->current = world->chunks.data[it->_progress++];
    } else {
        it->current = NULL;
    }
}

void world_chunks_clear_dirty(struct world *world) {
    int i;
    struct chunk *c;
    int to_clear = ~(CHUNK_FLAG_NEW | CHUNK_FLAG_DIRTY);
    vec_foreach(&world->chunks, c, i) {
            c->flags &= to_clear;
        }
}

int block_type_colour(enum block_type type) {
    switch (type) {
        case BLOCK_AIR: return 0xffffffff;
        case BLOCK_GROUND: return 0xffff0000;
        case BLOCK_OBJECT: return 0xff0000ff;
    }
}

int block_type_opaque(enum block_type type) {
    switch (type) {
        case BLOCK_AIR:
            return 0;
        default:
            return 1;
    }
}

void chunk_get_block_idx(struct chunk *chunk, int idx, struct block *out) {
   *out = chunk->blocks[idx];
}

void chunk_get_pos(struct chunk *chunk, ivec3 out) {
    glm_ivec_copy(chunk->pos, out);
}

inline void expand_flat_index(uint idx, ivec3 out) {
    int a_shift = CHUNK_WIDTH_SHIFT + CHUNK_HEIGHT_SHIFT;
    int a = 1 << a_shift;
    out[2] = idx >> a_shift;
    uint b = idx - a * out[2];
    out[1] = b >> CHUNK_WIDTH_SHIFT;
    out[0] = b & (CHUNK_WIDTH - 1);
}

// chunk is optional for hinted lookups
static void update_face_visibility_with_block(struct world *world, ivec3 pos,
                                              struct block *block, struct chunk *chunk) {
    int visibility = block->face_visibility;

    if (!block_type_opaque(block->type)) {
        // fully visibly
        visibility = FACE_VISIBILITY_ALL;
    } else {
        // check each face individually
        ivec3 offset;
        struct block *offset_block;
        for (int i = 0; i < FACE_COUNT; ++i) {
            face_offset(FACES[i], pos, offset);

            if ((offset_block = world_get_block_with_chunk_hint(world, chunk, offset))) {
                visibility = block_type_opaque(offset_block->type)
                             ? (visibility & ~(1 << i))
                             : (visibility | (1 << i));
            }
        }

    }

    block->face_visibility = visibility;
}

// lookups block and owning chunk from pos
static void update_face_visibility(struct world *world, ivec3 pos) {
    struct block *block = world_get_block(world, pos);
    if (!block)
        return;
    update_face_visibility_with_block(world, pos, block, NULL);
}

void chunk_init_lighting(struct world *world, struct chunk *chunk) {
    ivec3 chunk_pos, world_pos;
    struct block *block;
    for (uint i = 0; i < BLOCKS_PER_CHUNK; ++i) {
        expand_flat_index(i, chunk_pos);

        glm_ivec_copy(chunk_pos, world_pos);
        chunk_to_world_coords(chunk, world_pos);

        block = chunk_get_block(chunk, chunk_pos);
        update_face_visibility_with_block(world, world_pos, block, chunk);
    }
}
