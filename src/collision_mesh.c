#include "cglm/cglm.h"
#include "collision_mesh.h"
#include "chunk_mesh.h"
#include "world.h"
#include "log.h"
#include "vec.h"

#define vec_reserve_safely(vec, size, what) do { \
    vec_init(&(vec)); \
    vec_reserve(&(vec), size); \
    if (!(vec).data) { \
        LOG_WARN("failed to alloc " what); \
        return; \
    } \
} while (0);

#define vec_pusharr_vertex(vec, vertex) do { \
    vec_push((vec), vertex[0] * BLOCK_SIZE * 2); \
    vec_push((vec), vertex[1] * BLOCK_SIZE * 2); \
    vec_push((vec), vertex[2] * BLOCK_SIZE * 2); \
} while (0);

void collision_mesh_init(struct collision_mesh *mesh, struct chunk *chunk) {
    // thank you 0fps.net

    int dims[] = {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH};
    vec_int_t indices;
    vec_float_t vertices;
    vec_reserve_safely(vertices, 512, "vertices");
    vec_reserve_safely(indices, 512, "indices");

    // must be maximum of dimensions
    int mask[(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH)
             / GLM_MIN(CHUNK_WIDTH, GLM_MIN(CHUNK_HEIGHT, CHUNK_DEPTH))] = {0};

    // 3 dimensions
    for (int d = 0; d < 3; ++d) {
        int i, j, k, l, w, h;
        int u = (d + 1) % 3;
        int v = (d + 2) % 3;
        int x[3] = {0};
        ivec3 q = {0};

        q[d] = 1;
        for (x[d] = -1; x[d] < dims[d];) {
            // compute mask
            int n = 0;
            for (x[v] = 0; x[v] < dims[v]; ++x[v])
                for (x[u] = 0; x[u] < dims[u]; ++x[u]) {
                    int solid_a = chunk_is_solid_at(chunk, x);
                    int solid_b = chunk_is_solid_at(chunk, (ivec3) {x[0] + q[0], x[1] + q[1], x[2] + q[2]});
                    mask[n++] = (0 <= x[d] ? solid_a : 0) != (x[d] < dims[d] - 1 ? solid_b : 0);
                }
            ++x[d];

            // generate mesh
            n = 0;
            for (j = 0; j < dims[v]; ++j)
                for (i = 0; i < dims[u];) {
                    if (mask[n]) {
                        // compute width
                        for (w = 1; mask[n + w] && i + w < dims[u]; ++w);

                        // compute height
                        int done = 0;
                        for (h = 1; j + h < dims[v]; ++h) {
                            for (k = 0; k < w; ++k) {
                                if (!mask[n + k + h * dims[u]]) {
                                    done = 1;
                                    break;
                                }
                            }
                            if (done) {
                                break;
                            }
                        }

                        // add quad
                        x[u] = i;
                        x[v] = j;
                        ivec3 du = {0};
                        du[u] = w;
                        ivec3 dv = {0};
                        dv[v] = h;

                        ivec3 bl = {x[0], x[1], x[2]};
                        ivec3 tl = {x[0] + du[0], x[1] + du[1], x[2] + du[2]};
                        ivec3 tr = {x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2]};
                        ivec3 br = {x[0] + dv[0], x[1] + dv[1], x[2] + dv[2]};

                        int vs = vertices.length / 3;
                        int new_indices[6] = {
                                vs + 0, vs + 1, vs + 2,
                                vs + 2, vs + 3, vs + 0
                        };
                        vec_pusharr(&indices, new_indices, 6);

                        vec_pusharr_vertex(&vertices, bl);
                        vec_pusharr_vertex(&vertices, tl);
                        vec_pusharr_vertex(&vertices, tr);
                        vec_pusharr_vertex(&vertices, br);

                        // zero mask
                        for (l = 0; l < h; ++l)
                            for (k = 0; k < w; ++k) {
                                mask[n + k + l * dims[u]] = 0;
                            }

                        // inc and continue
                        i += w;
                        n += w;
                    } else {
                        ++i;
                        ++n;
                    }
                }
        }

    }

//    printf("%d INDICES: ", indices.length);
//    for (int x = 0; x < indices.length; ++x) {
//        printf("%d,", indices.data[x]);
//    }
//    puts("");
//    printf("%d vertices: ", vertices.length);
//    for (int x = 0; x < vertices.length; ++x) {
//        printf("%f,", ((union{float f; int i;})vertices.data[x]).f);
//    }
//    puts("");

    // done
    mesh->vertices = vertices.data; // taking ownership
    mesh->vertex_count = (unsigned int) (vertices.length / 3);
    mesh->indices = indices.data; // taking ownership
    mesh->tri_count = (unsigned int) (indices.length / 3);
}


void collision_mesh_dispose(struct collision_mesh *mesh) {
    if (mesh->indices) {
        free(mesh->indices);
        mesh->indices = NULL;
    }
    if (mesh->vertices) {
        free(mesh->vertices);
        mesh->vertices = NULL;
    }

    mesh->tri_count = mesh->vertex_count = 0;
    // proxy shape is deallocated and replaced in collision_mesh_attach
    // TODO refactor this if a collision mesh is ever disposed of and never reattached
}

void collision_mesh_attach(struct phys_world *world, struct collision_mesh *mesh, float chunk_pos[3]) {
    phys_update_chunk_collision_mesh(world, chunk_pos, &mesh->collision_state,
                                     mesh->vertices, mesh->vertex_count,
                                     mesh->indices, mesh->tri_count);
}
