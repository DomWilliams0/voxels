#include "voxel_game.h"
#include <stdlib.h>

int main(int argc, char **argv) {
    struct voxel_game game = {0};

    if (!game_init(&game, 800, 600)) {
        game_destroy(&game);
        exit(1);
    }

    game_start(&game, argc, argv);

    game_destroy(&game);
}