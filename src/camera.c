#include <SDL2/SDL.h>
#include "log.h"
#include "camera.h"

static void update(struct camera *camera) {
    versor q_pitch, q_yaw;
    glm_quatv(q_pitch, (camera->pitch), GLM_XUP);
    glm_quatv(q_yaw, (camera->yaw), GLM_YUP);

    versor orientation;
    mat4 rotation;
    glm_quat_mul(q_pitch, q_yaw, orientation);
    glm_quat_normalize(orientation);
    glm_quat_mat4(orientation, rotation);

    mat4 inv_rot;
    glm_mat4_inv_fast(rotation, inv_rot);
    glm_mat4_mulv(inv_rot, GLM_ZUP, camera->forward);
    glm_mat4_mulv(inv_rot, GLM_XUP, camera->right);

    mat4 translation = GLM_MAT4_IDENTITY_INIT;
    vec3 to_translate;
    glm_vec_inv_to(camera->pos, to_translate);
    glm_translate(translation, to_translate);

    glm_mul(rotation, translation, camera->transform);
}

void camera_init(struct camera *camera, vec3 start_pos, vec3 start_dir) {
    glm_vec_copy(start_pos, camera->pos);
    glm_mat4_identity(camera->transform);

    vec3 dir;
    glm_normalize_to(start_dir, dir);
    camera->yaw = atan2f(dir[2], dir[0]);
    camera->pitch = asinf(-dir[1]);

    update(camera);
}

void camera_turn(struct camera *camera, vec2 screen_delta) {

    camera->yaw += screen_delta[0] * CAMERA_TURN_SPEED;
    camera->pitch = glm_clamp(camera->pitch + screen_delta[1] * CAMERA_TURN_SPEED, -GLM_PI_2f, GLM_PI_2f);

    update(camera);
}


void camera_tick(struct camera *camera) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    int back = 0, right = 0;

    if (state[SDL_SCANCODE_W])
        back -= 1;
    if (state[SDL_SCANCODE_S])
        back += 1;
    if (state[SDL_SCANCODE_A])
        right -= 1;
    if (state[SDL_SCANCODE_D])
        right += 1;

    if (back == 0 && right == 0)
        return;

    vec3 to_move = GLM_VEC3_ZERO_INIT;
    vec3 tmp;

    // forward
    glm_vec_scale(camera->forward, back * CAMERA_MOVE_SPEED, tmp);
    glm_vec_add(to_move, tmp, to_move);

    // right
    glm_vec_scale(camera->right, right * CAMERA_MOVE_SPEED, tmp);
    glm_vec_add(to_move, tmp, to_move);

    glm_vec_add(camera->pos, to_move, camera->pos);
    update(camera);
}