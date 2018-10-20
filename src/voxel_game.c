#include <GLFW/glfw3.h>
#include "voxel_game.h"
#include "log.h"
#include "renderer.h"

static void error_callback(int error, const char *description) {
    LOG_ERROR("%d: %s", error, description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

ERR game_init(struct voxel_game *game, int width, int height) {
    if (!glfwInit()) {
        LOG_ERROR("failed to init glfw");
        return ERR_FAIL;
    }

    glfwSetErrorCallback(error_callback);
    // window config
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    if ((game->window = glfwCreateWindow(width, height, "OpenGL", 0, 0)) <= 0) {
        LOG_ERROR("failed to create window");
        return ERR_FAIL;
    }

    glfwSetKeyCallback(game->window, key_callback);
    glfwSetWindowSizeCallback(game->window, resize_callback);

    glfwMakeContextCurrent(game->window);
    glfwSwapInterval(1); // vsync
    glfwShowWindow(game->window);

    renderer_init(width, height);

    return ERR_SUCC;
}

void game_start(struct voxel_game *game) {

    while (!glfwWindowShouldClose(game->window)) {
        render();

        glfwSwapBuffers(game->window);
        glfwPollEvents();
    }
}

void game_destroy(struct voxel_game *game) {
    glfwDestroyWindow(game->window);
    game->window = NULL;
    glfwTerminate();
}
