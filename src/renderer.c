#include "renderer.h"
#include "cglm/cglm.h"


#define PI 3.141592f

#define deg_to_rad(deg) ((deg) * PI / 180.0f)
#define rad_to_deg(rad) ((rad * 180.0f / PI)

static int window_width, window_height;

void renderer_init(int width, int height) {
    window_width = width;
    window_height = height;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.1, 0.1, 0.3, 1.0);
}

void render() {
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

