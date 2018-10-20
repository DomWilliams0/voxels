#include <GLFW/glfw3.h>
#include "voxel_game.h"
#include "log.h"

ERR game_init(struct voxel_game *game, int width, int height) {
    if (!glfwInit()) {
        LOG_ERROR("failed to init glfw");
        return ERR_FAIL;
    }

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

    glfwMakeContextCurrent(game->window);

    // vsync
    glfwSwapInterval(1);

    glfwShowWindow(game->window);

    return ERR_SUCC;
}

void game_start(struct voxel_game *game) {

}

void game_destroy(struct voxel_game *game) {

}
