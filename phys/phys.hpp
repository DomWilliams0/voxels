#ifndef VOXEL_PHYS_H
#define VOXEL_PHYS_H

#ifdef __cplusplus
extern "C" {
#endif

struct phys_world;
struct phys_dyn_voxel {
    void *rigid_body;
    void *last_transform;
};

// TODO vertex array params
void phys_world_init(struct phys_world **world);

void phys_world_destroy(struct phys_world *world);

void phys_world_tick(struct phys_world *world, double dt);

struct phys_dyn_voxel phys_add_body(struct phys_world *world, float pos[3]);

void phys_get_body_transform(struct phys_dyn_voxel *dyn_voxel, float *mat, float interpolation);


#ifdef __cplusplus
}
#endif

#endif

