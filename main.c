#include "voxel_game.h"
#include <stdlib.h>

int main() {
    struct voxel_game game;

    if (!game_init(&game, 800, 600)) {
        exit(1);
    }

    game_start(&game);

    game_destroy(&game);
}