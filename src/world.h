#ifndef VOXEL_GAME_WORLD_H
#define VOXEL_GAME_WORLD_H

#include "error.h"

#define CHUNK_WIDTH_SHIFT 5
#define CHUNK_HEIGHT_SHIFT 5

#define CHUNK_WIDTH (1<<CHUNK_WIDTH_SHIFT)
#define CHUNK_DEPTH (CHUNK_WIDTH)
#define CHUNK_HEIGHT (1<<CHUNK_HEIGHT_SHIFT)

struct world;

ERR world_load_demo(struct world **world, const char *name);

void world_destroy(struct world *world);

#endif
