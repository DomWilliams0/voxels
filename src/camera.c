#include "log.h"
#include "camera.h"

static void update(struct camera *camera) {
    versor q_pitch, q_yaw;
    glm_quatv(q_pitch, (camera->pitch), (vec3) {1, 0, 0});
    glm_quatv(q_yaw, (camera->yaw), (vec3) {0, 1, 0});

    versor orientation;
    mat4 rotation;
    glm_quat_mul(q_pitch, q_yaw, orientation);
    glm_quat_normalize(orientation);
    glm_quat_mat4(orientation, rotation);

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
    float speed = 0.01;

    camera->yaw += screen_delta[0] * speed;
    camera->pitch = glm_clamp(camera->pitch + screen_delta[1] * speed, -GLM_PI_2f, GLM_PI_2f);

    update(camera);
}

