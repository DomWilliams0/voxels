#include <cglm/cglm.h>
#include <vec.h>
// TODO consistent style in including libs, <> or ""
#include "world.h"
#include "log.h"
#include "chunk_mesh.h"
#include "face.h"
#include "phys.hpp"

struct chunk {
    ivec3 pos;
    int flags;
    struct chunk_render_objs render_objs;
    struct chunk_mesh_meta mesh;

    struct block blocks[BLOCKS_PER_CHUNK];

};

typedef vec_t(struct chunk *) chunk_vec_t;

struct world {
    struct phys_world *phys_world;
    struct dyn_voxel dynamic_voxels[MAX_DYN_VOXELS];
    unsigned int dynamic_voxel_count;

    ivec3 max_block_dims;
    ivec3 max_chunk_dims;
    size_t chunk_count;
    struct chunk **chunks_arr; // TODO rename once all errors fixed
    chunk_vec_t loaded_chunks;
};

// helper
static inline void glm_ivec_copy(const ivec3 src, ivec3 dst) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

// helper
static inline void glm_ivec_add(const ivec3 src, const ivec3 add, ivec3 dst) {
    dst[0] = src[0] + add[0];
    dst[1] = src[1] + add[1];
    dst[2] = src[2] + add[2];
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
        chunk->render_objs.vao = 0;
        chunk->render_objs.vbo = 0;
        chunk->flags = CHUNK_FLAG_NEW | CHUNK_FLAG_DIRTY | CHUNK_FLAG_VISIBLE;
        glm_ivec_zero(chunk->pos);

        struct block default_block = {
                .type = BLOCK_AIR,
                .face_visibility = FACE_VISIBILITY_ALL,
                .ao = AO_BLOCK_NONE,
        };
        for (int i = 0; i < BLOCKS_PER_CHUNK; ++i) {
            chunk->blocks[i] = default_block;
        }
    }
    return chunk;
}

static ERR world_init(struct world *world, ivec3 max_block_dims) {
    // round to nearest multiple of chunk size
    max_block_dims[0] = (max_block_dims[0] >> CHUNK_WIDTH_SHIFT) << CHUNK_WIDTH_SHIFT;
    max_block_dims[1] = (max_block_dims[1] >> CHUNK_HEIGHT_SHIFT) << CHUNK_HEIGHT_SHIFT;
    max_block_dims[2] = (max_block_dims[2] >> CHUNK_WIDTH_SHIFT) << CHUNK_WIDTH_SHIFT;

    if (max_block_dims[0] <= 0 ||
        max_block_dims[1] <= 0 ||
        max_block_dims[2] <= 0) {
        LOG_ERROR("world size limit cannot <= 0");
        return ERR_FAIL;
    }


    phys_world_init(&world->phys_world);
    if (world->phys_world == NULL) {
        LOG_ERROR("failed to allocate physics world")
        return ERR_FAIL;
    }
    world->dynamic_voxel_count = 0;
    memset(world->dynamic_voxels, 0, sizeof(world->dynamic_voxels));


    glm_ivec_copy(max_block_dims, world->max_block_dims);
    world->max_chunk_dims[0] = max_block_dims[0] / CHUNK_WIDTH;
    world->max_chunk_dims[1] = max_block_dims[1] / CHUNK_HEIGHT;
    world->max_chunk_dims[2] = max_block_dims[2] / CHUNK_DEPTH;

    size_t chunk_count = ((size_t) world->max_chunk_dims[0]) *
                         ((size_t) world->max_chunk_dims[1]) *
                         ((size_t) world->max_chunk_dims[2]);
    world->chunk_count = chunk_count;
    // TODO check if overflowed

    vec_init(&world->loaded_chunks);

    if ((world->chunks_arr = calloc(chunk_count, sizeof(void *))) == NULL) {
        LOG_ERROR("failed to allocate chunks array of size %lu", chunk_count);
        world_destroy(world);
        return ERR_FAIL;
    }

    return ERR_SUCC;
}

inline static int world_get_chunk_index(struct world *world, const ivec3 chunk_pos) {
    return (world->max_chunk_dims[0] * world->max_chunk_dims[1] * chunk_pos[2]) +
           (world->max_chunk_dims[0] * chunk_pos[1]) + chunk_pos[0];
}

inline static int world_is_chunk_pos_in_range(struct world *world, const ivec3 chunk_pos) {
    return chunk_pos[0] < world->max_chunk_dims[0] &&
           chunk_pos[1] < world->max_chunk_dims[1] &&
           chunk_pos[2] < world->max_chunk_dims[2];
}

// expects pos to be in range
static struct chunk *world_add_chunk(struct world *world, ivec3 pos) {
    struct chunk *c = chunk_alloc();
    if (c) {
        glm_ivec_copy(pos, c->pos);
        world->chunks_arr[world_get_chunk_index(world, pos)] = c;
        vec_push(&world->loaded_chunks, c);
    }
    return c;
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

static struct chunk *world_find_chunk(struct world *world, const ivec3 block_pos, int *was_out_of_range) {
    ivec3 chunk_pos = IVEC3_COPY(block_pos);
    resolve_chunk_coords(chunk_pos);

    if (!world_is_chunk_pos_in_range(world, chunk_pos)) {
        if (was_out_of_range) *was_out_of_range = 1;
        return NULL;
    }

    int idx = world_get_chunk_index(world, chunk_pos);
    if (idx < 0 || idx >= world->chunk_count)
        return NULL;

    return world->chunks_arr[idx];
}

#define CHUNK_IDX(pos) ((CHUNK_WIDTH * CHUNK_HEIGHT * pos[2]) + (CHUNK_WIDTH * pos[1]) + pos[0])

// assumes pos is in the range of a chunk
static struct block *chunk_get_block_unsafe(struct chunk *chunk, const ivec3 pos) {
    return &chunk->blocks[CHUNK_IDX(pos)];
}

// checks if pos in in the range of a chunk
static struct block *chunk_get_block(struct chunk *chunk, const ivec3 pos) {
    int idx = CHUNK_IDX(pos);
    if (idx < 0 || idx >= BLOCKS_PER_CHUNK) return NULL;

    return &chunk->blocks[idx];
}

static struct block *world_get_block(struct world *world, const ivec3 pos) {
    struct chunk *chunk = world_find_chunk(world, pos, NULL);
    if (!chunk) return NULL;

    ivec3 chunk_local_pos = IVEC3_COPY(pos);
    world_to_chunk_coords(chunk_local_pos);

    return chunk_get_block_unsafe(chunk, chunk_local_pos);
}

// hint can be NULL
static struct block *world_get_block_with_chunk_hint(struct world *world, struct chunk *hint, const ivec3 pos) {
    if (hint) {
        ivec3 chunk_pos = IVEC3_COPY(pos);
        resolve_chunk_coords(chunk_pos);

        if (glm_ivec_eq(chunk_pos, hint->pos)) {
            ivec3 chunk_local_pos = IVEC3_COPY(pos);
            world_to_chunk_coords(chunk_local_pos);

            return chunk_get_block_unsafe(hint, chunk_local_pos);
        }
    }

    return world_get_block(world, pos);
}

static void demo_set_block_safely(struct world *world, ivec3 pos, enum block_type type) {
    struct block *b = NULL;
    struct chunk *c = NULL;

    // attempt to get block
    b = world_get_block(world, pos);

    if (!b) {
        // check if chunk is out of range
        int out_of_range = 0;
        c = world_find_chunk(world, pos, &out_of_range);

        if (out_of_range) {
            LOG_WARN("block %d,%d,%d is outside of world's boundaries %d,%d,%d",
                     pos[0], pos[1], pos[2],
                     world->max_block_dims[0], world->max_block_dims[1], world->max_block_dims[2])
            return;
        }

            // chunk is in range but doesn't exist yet
        else if (!c) {
            ivec3 chunk_coord = IVEC3_COPY(pos);
            resolve_chunk_coords(chunk_coord);
            c = world_add_chunk(world, chunk_coord);
            if (c) {
                world_to_chunk_coords(pos);
                b = chunk_get_block_unsafe(c, pos);

            }
        } else {
            LOG_WARN("something went wrong");
        }
    }

    if (b) b->type = type;
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

static void world_add_voxel_body(struct world *world, const ivec3 pos, enum block_type type) {
    // limit reached
    if (world->dynamic_voxel_count == MAX_DYN_VOXELS) {
        LOG_WARN("max dynamic voxel body limit reached, cant spawn any more (%d)", MAX_DYN_VOXELS);
        return;
    }

    // create body
    vec3 world_pos = {
            (float) pos[0] * BLOCK_SIZE,
            (float) pos[1] * BLOCK_SIZE,
            (float) pos[2] * BLOCK_SIZE,
    };

    struct dyn_voxel *dyn = &world->dynamic_voxels[world->dynamic_voxel_count++];
    dyn->type = type;
    dyn->phys_body = phys_add_body(world->phys_world, world_pos);
}

ERR world_load_demo(struct world **world, const char *name) {
    struct world *w = malloc(sizeof(struct world));
    if (!w) {
        LOG_ERROR("failed to allocate world");
        return ERR_FAIL;
    }

    // TODO test this actual limit
    ivec3 demo_block_limit = {1000, 1000, 1000};

    if (!world_init(w, demo_block_limit)) {
        free(w);
        return ERR_FAIL;
    }

    int result = ERR_SUCC;


    if (!strcmp(name, "demo")) {
        demo_fill_range(w, (ivec3) {0, 0, 0}, (ivec3) {10, 1, 10}, BLOCK_GROUND);

        // pillar
        demo_fill_range(w, (ivec3) {3, 0, 3}, (ivec3) {1, 5, 1}, BLOCK_OBJECT);

        demo_set_block_safely(w, (ivec3) {4, 1, 3}, BLOCK_OBJECT);
        demo_set_block_safely(w, (ivec3) {3, 1, 4}, BLOCK_OBJECT);
        demo_set_block_safely(w, (ivec3) {2, 1, 3}, BLOCK_OBJECT);
        demo_set_block_safely(w, (ivec3) {3, 1, 2}, BLOCK_OBJECT);

        demo_set_block_safely(w, (ivec3) {4, 4, 3}, BLOCK_OBJECT);
        demo_set_block_safely(w, (ivec3) {3, 4, 4}, BLOCK_OBJECT);
        demo_set_block_safely(w, (ivec3) {2, 4, 3}, BLOCK_OBJECT);
        demo_set_block_safely(w, (ivec3) {3, 4, 2}, BLOCK_OBJECT);

        // falling test block
        world_add_voxel_body(w, (ivec3) {0, 2, 0}, BLOCK_OBJECT);


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
        if (world->chunks_arr) {
            struct chunk *c;
            for (int i = 0; i < world->chunk_count; ++i) {
                c = world->chunks_arr[i];
                if (c) free(c);
            }
            free(world->chunks_arr);
            world->chunks_arr = NULL;
        }

        if (world->phys_world) {
            phys_world_destroy(world->phys_world);
            world->phys_world = NULL;
        }
    }
}

void world_tick_physics(struct world *world, double dt) {
    phys_world_tick(world->phys_world, dt);
}

int chunk_has_flag(struct chunk *chunk, enum chunk_flag flag) {
    return (chunk->flags & flag) == flag;
}

struct chunk_render_objs *chunk_render_objs(struct chunk *chunk) {
    return &chunk->render_objs;
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
    if (world->loaded_chunks.length > 0) {
        it->_progress = 1;
        it->current = vec_first(&world->loaded_chunks);
    } else {
        it->current = NULL;
    }
}

void world_chunks_next(struct world *world, struct chunk_iterator *it) {
    if (it->_progress < world->loaded_chunks.length) {
        it->current = world->loaded_chunks.data[it->_progress++];
    } else {
        it->current = NULL;
    }
}

void world_chunks_clear_dirty(struct world *world) {
    int i;
    struct chunk *c;
    int to_clear = ~(CHUNK_FLAG_NEW | CHUNK_FLAG_DIRTY);
    vec_foreach(&world->loaded_chunks, c, i) {
            c->flags &= to_clear;
        }
}

int block_type_colour(enum block_type type) {
    switch (type) {
        case BLOCK_AIR: return 0xffffffff;
        case BLOCK_GROUND: return 0xffe08d3b;
        case BLOCK_OBJECT: return 0xff577fda;
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

static const enum face face_ao_definitions[FACE_COUNT][1 + 8] = {
        {FACE_TOP,
                FACE_FRONT,  FACE_LEFT,
                FACE_FRONT,  FACE_RIGHT,
                FACE_BACK,   FACE_RIGHT,
                FACE_BACK,   FACE_LEFT
        },
        {FACE_BOTTOM,
                FACE_BACK,   FACE_LEFT,
                FACE_BACK,   FACE_RIGHT,
                FACE_FRONT,  FACE_RIGHT,
                FACE_FRONT,  FACE_LEFT
        },

        {FACE_FRONT,
                FACE_BOTTOM, FACE_LEFT,
                FACE_BOTTOM, FACE_RIGHT,
                FACE_TOP,    FACE_RIGHT,
                FACE_TOP,    FACE_LEFT
        },
        {FACE_BACK,
                FACE_TOP,    FACE_LEFT,
                FACE_TOP,    FACE_RIGHT,
                FACE_BOTTOM, FACE_RIGHT,
                FACE_BOTTOM, FACE_LEFT
        },

        {FACE_LEFT,
                FACE_BOTTOM, FACE_BACK,
                FACE_BOTTOM, FACE_FRONT,
                FACE_TOP,    FACE_FRONT,
                FACE_TOP,    FACE_BACK
        },

        {FACE_RIGHT,
                FACE_BOTTOM, FACE_FRONT,
                FACE_BOTTOM, FACE_BACK,
                FACE_TOP,    FACE_BACK,
                FACE_TOP,    FACE_FRONT
        }
};

inline static int is_opaque_with_default_nope(struct world *world, struct chunk *chunk, const ivec3 block_pos) {
    struct block *b = world_get_block_with_chunk_hint(world, chunk, block_pos);
    return b ? block_type_opaque(b->type) : 0;
}

inline static char calculate_vertex_ao(struct world *world, struct chunk *chunk, const ivec3 block_pos,
                                       enum face face, enum face a, enum face b) {
    ivec3 acc;
    face_offset(face, block_pos, acc);

    ivec3 v0, v1, v2;
    face_offset(a, acc, acc);
    glm_ivec_copy(acc, v0);

    face_offset(b, acc, acc);
    glm_ivec_copy(acc, v1);

    face_offset(face_opposite(a), acc, acc);
    glm_ivec_copy(acc, v2);

    int s1 = is_opaque_with_default_nope(world, chunk, v0);
    int s2 = is_opaque_with_default_nope(world, chunk, v2);
    int corner = is_opaque_with_default_nope(world, chunk, v1);

    return (char) (s1 && s2 ? AO_VERTEX_FULL : (AO_VERTEX_NONE - (s1 + s2 + corner)));
}

// chunk is optional for hinted lookups
static void update_lighting_with_block(struct world *world, const ivec3 pos,
                                       struct block *block, struct chunk *chunk) {
    int visibility = block->face_visibility;
    long ao = 0;

    if (!block_type_opaque(block->type)) {
        // fully visibly and not occluded
        visibility = FACE_VISIBILITY_ALL;
        ao = AO_BLOCK_NONE;
    } else {
        // check each face individually for visibility
        {
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

        // ao
        {
            for (int i = 0; i < FACE_COUNT; ++i) {
                const enum face *definition = face_ao_definitions[i];
                enum face face = definition[0];

                char v05 = calculate_vertex_ao(world, chunk, pos, face, definition[1], definition[2]);
                char v1 = calculate_vertex_ao(world, chunk, pos, face, definition[3], definition[4]);
                char v23 = calculate_vertex_ao(world, chunk, pos, face, definition[5], definition[6]);
                char v4 = calculate_vertex_ao(world, chunk, pos, face, definition[7], definition[8]);
                ao |= ao_set_face(face, v05, v1, v23, v4);
            }
        }

    }

    block->face_visibility = visibility;
    block->ao = ao;
}

// lookups block and owning chunk from pos
static void update_lighting(struct world *world, const ivec3 pos) {
    struct block *block = world_get_block(world, pos);
    if (!block)
        return;
    update_lighting_with_block(world, pos, block, NULL);
}

void chunk_init_lighting(struct world *world, struct chunk *chunk) {
    ivec3 chunk_pos, world_pos;
    struct block *block;
    for (uint i = 0; i < BLOCKS_PER_CHUNK; ++i) {
        expand_flat_index(i, chunk_pos);

        glm_ivec_copy(chunk_pos, world_pos);
        chunk_to_world_coords(chunk, world_pos);

        block = chunk_get_block_unsafe(chunk, chunk_pos);
        update_lighting_with_block(world, world_pos, block, chunk);
    }
}

long ao_set_face(enum face face, char v05, char v1, char v23, char v4) {
    long byte =
            v05 |
            v1 << 2 |
            v23 << 4 |
            v4 << 6;

    int shift = (int) face * 8;
    return byte << shift;
}

char ao_get_vertex(long ao, enum face face, int vertex_idx) {
    int real_idx;
    switch (vertex_idx) {
        case 0:
        case 5:
            real_idx = 0;
            break;
        case 1:
            real_idx = 1;
            break;
        case 2:
        case 3:
            real_idx = 2;
            break;
        case 4:
            real_idx = 3;
            break;
        default:
            LOG_ERROR("bad ao vertex index %d, should be < 6", vertex_idx);
            return 0; // terrible default value
    }

    int byte_mask_shift = face * 8;
    long byte_mask = (1 << 8) - 1;
    long shifted_mask = byte_mask << byte_mask_shift;
    long byte = (ao & shifted_mask) >> byte_mask_shift;

    int vertex_shift = real_idx * 2;
    int vertex_mask = 3 << vertex_shift;

    long result = (byte & vertex_mask) >> vertex_shift;
    return (char) result;
}

void world_set_block(struct world *world, ivec3 pos, enum block_type type) {
    struct chunk *chunk = world_find_chunk(world, pos, NULL);
    if (!chunk) return;

    struct block *block = world_get_block_with_chunk_hint(world, chunk, pos);
    if (!block) return;

    block->type = type;

    // dirty flag to regen chunk mesh next frame
    chunk->flags |= CHUNK_FLAG_DIRTY;

    // recalculate visibility and lighting for all blocks around
    // TODO
    ivec3 offset;
    struct block *offset_block;
    for (int z = -1; z <= 1; ++z) {
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                glm_ivec_add(pos, (ivec3) {x, y, z}, offset);
                if ((offset_block = world_get_block_with_chunk_hint(world, chunk, offset)))
                    update_lighting_with_block(world, offset, offset_block, chunk);
            }
        }
    }
}

static int dyn_voxels_find_next(struct world *world, int start, struct dyn_voxel **out) {

    for (int i = start+1; i < MAX_DYN_VOXELS; ++i) {
        struct dyn_voxel *dyn = &world->dynamic_voxels[i];
        if (dyn->phys_body.rigid_body != NULL) {
            *out = dyn;
            return i;
        }
    }
    *out = NULL;
    return 0;
}

int world_dyn_voxels_next(struct world *world, struct dyn_voxel_iterator *it) {
    it->valid = 0;

    it->_progress = dyn_voxels_find_next(world, it->_progress, &it->current);

    if (it->current != NULL) {
       it->valid = 1;
    }

    return it->valid;
}

