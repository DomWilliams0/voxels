#include <GL/glew.h>

#include "voxel_game.h"
#include "log.h"
#include "renderer.h"
#include "camera.h"
#include "world.h"

static int game_running = 1;

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

static void game_tick(struct voxel_game *game, struct camera *camera);

void game_start(struct voxel_game *game, int argc, char **argv) {
    struct world *world;
    char *demo_name = argc > 1 ? argv[1] : "demo";
    if (!world_load_demo(&world, demo_name)) {
        LOG_ERROR("failed to load world '%s'", demo_name);
        return;
    }
    game->renderer.world = world;

    struct camera camera;
    camera_init(&camera, (vec3) {-2, 0, 0}, (vec3) {-0.3f, 0, 1});

    int next_tick = SDL_GetTicks();
    int loops;
    float interpolation;

    SDL_Event evt;
    while (game_running) {
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_QUIT:
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    switch (evt.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            if (evt.key.type == SDL_KEYDOWN) game_running = 0;
                            break;

                        case SDLK_y:
                            if (evt.key.type == SDL_KEYDOWN) renderer_toggle_wireframe(&game->renderer);
                            break;

                        default:
                            break;
                    }
                }
                    break;

                case SDL_MOUSEMOTION:
                    if (evt.motion.state & SDL_BUTTON_LMASK)
                        camera_turn(&camera, (vec2) {evt.motion.xrel, evt.motion.yrel});
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if (evt.button.button == SDL_BUTTON_LEFT)
                        SDL_SetRelativeMouseMode(evt.button.state == SDL_PRESSED ? SDL_TRUE : SDL_FALSE);

                case SDL_WINDOWEVENT:
                    if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
                        resize_callback(evt.window.data1, evt.window.data2);
                    break;

                default:
                    break;
            }
        }

        // merci http://www.koonsolo.com/news/dewitters-gameloop/
        const int TICKS_PER_SECOND = 20;
        const int SKIP_TICKS = 1000 / TICKS_PER_SECOND;
        const int MAX_FRAMESKIP = 5;

        loops = 0;
        while (SDL_GetTicks() > next_tick && loops < MAX_FRAMESKIP) {
            game_tick(game, &camera);

            next_tick += SKIP_TICKS;
            loops += 1;
        }

        interpolation = ((float) (SDL_GetTicks() + SKIP_TICKS - next_tick)) / SKIP_TICKS;

        render(&game->renderer, &camera, interpolation);
        SDL_GL_SwapWindow(game->window);
    }

    world_destroy(world);
    game->renderer.world = NULL; // TODO destroy renderer?
}

static void game_tick(struct voxel_game *game, struct camera *camera) {
    camera_tick(camera);
}

void game_destroy(struct voxel_game *game) {
    if (game->window) {
        SDL_DestroyWindow(game->window);
        game->window = NULL;
    }
}
