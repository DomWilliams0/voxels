#ifndef VOXEL_PHYS_H
#define VOXEL_PHYS_H

#ifdef __cplusplus
extern "C" {
#endif

struct phys_world;

// TODO vertex array params
void phys_world_init(struct phys_world **world);

void phys_world_destroy(struct phys_world *world);

void phys_world_tick(struct phys_world *world, double dt);

void *phys_add_body(struct phys_world *world, float pos[3]);

void phys_get_body_transform(void *body, float mat[16]);


#ifdef __cplusplus
}
#endif

#endif

