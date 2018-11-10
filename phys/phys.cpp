#include "phys.hpp"
#include "reactphysics3d.h"

#include <iostream>

using namespace reactphysics3d;

#ifdef __cplusplus
extern "C" {
#endif

struct phys_world {
    DynamicsWorld dyn_world;

    phys_world() : dyn_world(Vector3(0, -9.81f, 0)) {}
};


void phys_world_init(struct phys_world **world) {
    *world = new(std::nothrow) phys_world;
}

void phys_world_destroy(struct phys_world *world) {
    delete world;
}

void phys_world_tick(struct phys_world *world, double dt) {
    world->dyn_world.update(static_cast<decimal>(dt));
}

struct phys_voxel_body phys_add_body(struct phys_world *world, float pos[3]) {
    Vector3 position(pos[0], pos[1], pos[2]);
    Quaternion rot = Quaternion::identity(); // TODO ever need to specify rotation?
    Transform transform(position, rot);

    auto body = world->dyn_world.createRigidBody(transform);
    body->setType(BodyType::DYNAMIC);
    // TODO material

    // TODO add collisions

    return phys_voxel_body{._rigid_body = body};
}

void phys_get_body_transform(struct phys_voxel_body *body, float mat[16]) {
    // TODO interpolate
    auto *rb = static_cast<RigidBody *>(body->_rigid_body);
    rb->getTransform().getOpenGLMatrix(mat);
}

#ifdef __cplusplus
}
#endif
