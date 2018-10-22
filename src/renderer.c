#include "renderer.h"
#include "cglm/cglm.h"
#include "world.h"


#define PI 3.141592f

#define deg_to_rad(deg) ((deg) * PI / 180.0f)
#define rad_to_deg(rad) ((rad * 180.0f / PI)

static int window_width, window_height;

void renderer_init(struct renderer *renderer, int width, int height) {
    window_width = width;
    window_height = height;
    renderer->world = NULL;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.1, 0.1, 0.3, 1.0);

    glGenVertexArrays(1, &renderer->world_vao);
}

static void update_chunks(struct renderer *renderer) {
    struct chunk_iterator it;
    world_chunks_first(renderer->world, &it);

    while (it.current) {
        int *vbo = chunk_vbo(it.current);

        if (chunk_has_flag(it.current, CHUNK_FLAG_NEW)) {
            glGenBuffers(1, vbo);
        }

        if (chunk_has_flag(it.current, CHUNK_FLAG_DIRTY)) {
            // TODO generate mesh
//            glBindBuffer(GL_ARRAY_BUFFER, *vbo)
//            glBufferData(GL_ARRAY_BUFFER, mesh, GL_STATIC_DRAW)
        }

        world_chunks_next(renderer->world, &it);
    }

    // TODO flags are needed in tick too
    world_chunks_clear_dirty(renderer->world);
}

void render(struct renderer *renderer) {
    update_chunks(renderer);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);

    mat4 proj;
    glm_perspective(
            deg_to_rad(45),
            ((float) window_width) / window_height,
            0.1,
            50,
            (vec4 *) &proj);
}

void resize_callback(GLFWwindow *window, int width, int height) {
    window_width = width;
    window_height = height;
}

