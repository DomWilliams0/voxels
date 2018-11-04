#include <SDL2/SDL.h>
#include "log.h"
#include "camera.h"

// updates transform
static void update_transform(struct camera_state *state) {
    mat4 rotation;
    glm_quat_mat4(state->rot, rotation);

    mat4 translation = GLM_MAT4_IDENTITY_INIT;
    vec3 to_translate;
    glm_vec_inv_to(state->pos, to_translate);
    glm_translate(translation, to_translate);

    glm_mul(rotation, translation, state->transform);
}

// updates forward and right
static void update_directions(struct camera_state *state) {
    mat4 rotation;
    glm_quat_mat4(state->rot, rotation);

    mat4 inv_rot;
    glm_mat4_inv_fast(rotation, inv_rot);
    glm_mat4_mulv(inv_rot, GLM_ZUP, state->forward);
    glm_mat4_mulv(inv_rot, GLM_XUP, state->right);
}

// updates orientation from pitch and yaw
static void update_orientation(struct camera *camera) {
    versor q_pitch, q_yaw;
    glm_quatv(q_pitch, (camera->pitch), GLM_XUP);
    glm_quatv(q_yaw, (camera->yaw), GLM_YUP);

    versor orientation;
    glm_quat_mul(q_pitch, q_yaw, orientation);
    glm_quat_normalize(orientation);

    glm_quat_copy(orientation, camera->state.rot);
}

void camera_init(struct camera *camera, vec3 start_pos, vec3 start_dir) {
    glm_vec_copy(start_pos, camera->state.pos);
    glm_mat4_identity(camera->state.transform);

    vec3 dir;
    glm_normalize_to(start_dir, dir);
    camera->yaw = atan2f(dir[2], dir[0]);
    camera->pitch = asinf(-dir[1]);

    update_orientation(camera);
}

void camera_turn(struct camera *camera, vec2 screen_delta) {
    camera->yaw += screen_delta[0] * CAMERA_TURN_SPEED;
    camera->pitch = glm_clamp(camera->pitch + screen_delta[1] * CAMERA_TURN_SPEED, -GLM_PI_2f, GLM_PI_2f);

    update_orientation(camera);
}


void camera_tick(struct camera *camera, double dt) {
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

    float dtf = (float) dt;
    vec3 to_move = GLM_VEC3_ZERO_INIT;
    vec3 tmp;

    update_directions(&camera->state);

    // forward
    glm_vec_scale(camera->state.forward, back * CAMERA_MOVE_SPEED * dtf, tmp);
    glm_vec_add(to_move, tmp, to_move);

    // right
    glm_vec_scale(camera->state.right, right * CAMERA_MOVE_SPEED * dtf, tmp);
    glm_vec_add(to_move, tmp, to_move);

    glm_vec_add(camera->state.pos, to_move, camera->state.pos);
    update_transform(&camera->state);
}

void camera_interpolate(struct camera_state *last, struct camera_state *new, double alpha, struct camera_state *out) {
    float lerp = (float) alpha;
    glm_vec_lerp(last->pos, new->pos, lerp, out->pos);
    glm_quat_slerp(last->rot, new->rot, lerp, out->rot);

    update_transform(out);
}
