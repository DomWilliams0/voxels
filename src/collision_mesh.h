#ifndef VOXEL_GAME_COLLISION_MESH_H
#define VOXEL_GAME_COLLISION_MESH_H

struct collision_mesh {
    float *vertices;
    int *indices;
    unsigned int tri_count;
    unsigned int vertex_count;
    void *collision_state;
};

struct chunk;
void collision_mesh_init(struct collision_mesh *mesh, struct chunk *chunk);

void collision_mesh_dispose(struct collision_mesh *mesh);

struct phys_world;
void collision_mesh_attach(struct phys_world *world, struct collision_mesh *mesh, float chunk_pos[3]);


#endif
