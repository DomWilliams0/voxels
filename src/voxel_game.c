#include <GL/glew.h>

#include "voxel_game.h"
#include "log.h"
#include "renderer.h"
#include "world.h"

static int game_running = 1;

static void key_callback(int is_down, int key) {
    if (is_down && key == SDLK_ESCAPE)
        game_running = 0;
}

ERR game_init(struct voxel_game *game, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_ERROR("failed to init sdl: %s", SDL_GetError());
        return ERR_FAIL;
    }

    // window config
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    if ((game->window = SDL_CreateWindow("OpenGL",
                                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                         width, height, SDL_WINDOW_OPENGL)) <= 0) {
        LOG_ERROR("failed to create window");
        return ERR_FAIL;
    }

    SDL_GLContext *gl = SDL_GL_CreateContext(game->window);
    SDL_GL_MakeCurrent(game->window, gl);
    if (glewInit() != GLEW_OK) {
        LOG_ERROR("failed to init glew");
        return ERR_FAIL;
    }

    if (!renderer_init(&game->renderer, width, height))
        return ERR_FAIL;

    return ERR_SUCC;
}

void game_start(struct voxel_game *game) {
    struct world *world;
    if (!world_load_demo(&world, "demo")) {
        LOG_ERROR("failed to load world");
        return;
    }

    game->renderer.world = world;

    SDL_Event evt;
    while (game_running) {
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_QUIT:
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    key_callback(evt.key.type == SDL_KEYDOWN, evt.key.keysym.sym);

                case SDL_WINDOWEVENT:
                    if (evt.window.type == SDL_WINDOWEVENT_RESIZED)
                        resize_callback(evt.window.data2, 0);

                default:
                    break;
            }
        }

        render(&game->renderer);

        SDL_GL_SwapWindow(game->window);
    }

    world_destroy(world);
    game->renderer.world = NULL; // TODO destroy renderer?
}

void game_destroy(struct voxel_game *game) {
    if (game->window) {
        SDL_DestroyWindow(game->window);
        game->window = NULL;
    }
}
