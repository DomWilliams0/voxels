#include "phys.hpp"
#include "reactphysics3d.h"

#include <iostream>

using namespace reactphysics3d;

#ifdef __cplusplus
extern "C" {
#endif

struct phys_world {
    DynamicsWorld dyn_world;
    RigidBody *world_body;

    // reduced gravity to stop things falling so fast and moving through the ground
    phys_world() : dyn_world(Vector3(0, -6.81f, 0)) {
        world_body = dyn_world.createRigidBody(Transform::identity());
        world_body->setType(BodyType::STATIC);
        world_body->getMaterial().setBounciness(0.1);
    }

    ~phys_world() {
        dyn_world.destroyRigidBody(world_body);
    }
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

extern const float block_size;

struct phys_dyn_voxel phys_add_body(struct phys_world *world, float pos[3]) {
    Vector3 position(pos[0], pos[1], pos[2]);
    Quaternion rot = Quaternion::identity(); // TODO ever need to specify rotation?
    Transform transform(position, rot);

    auto body = world->dyn_world.createRigidBody(transform);
    body->setType(BodyType::DYNAMIC);
    body->getMaterial().setBounciness(0.2);
    // TODO material

    // add collisions
    static BoxShape shared_box_shape(Vector3(block_size, block_size, block_size));
    Vector3 box_offset(block_size, block_size, block_size);
    const float mass = 1;
    ProxyShape *ps = body->addCollisionShape(&shared_box_shape, Transform(box_offset, Quaternion::identity()), mass);
    // TODO add ps to output so it can be removed

    return phys_dyn_voxel{
            .rigid_body = body,
            .last_transform = new Transform,
    };
}

void phys_get_body_transform(struct phys_dyn_voxel *dyn_voxel, float *mat, float interpolation) {
    auto *rb = static_cast<RigidBody *>(dyn_voxel->rigid_body);
    auto &current_transform = rb->getTransform();
    auto *last_transform = static_cast<Transform *>(dyn_voxel->last_transform);

    // interpolate
    Transform::interpolateTransforms(
            *last_transform,
            current_transform,
            interpolation
    ).getOpenGLMatrix(mat);

    // update last_transform
    *static_cast<Transform *>(dyn_voxel->last_transform) = current_transform;
}

struct pointer_bundle {
    TriangleVertexArray *tva;
    TriangleMesh *tri_mesh;
    ConcaveMeshShape *mesh;
    ProxyShape *proxy_shape;
};

void *phys_update_chunk_collision_mesh(struct phys_world *world, float *offset, void **old_state,
                                       float *vertices, uint vertex_count,
                                       int *indices, uint triangle_count) {

    pointer_bundle *pointers;
    if (*old_state) {
        pointers = static_cast<pointer_bundle *>(*old_state);
        // TODO remove old subpart only, unnecessary deallocation here
        delete (pointers->tva);
        delete (pointers->tri_mesh);
        delete (pointers->mesh);
        world->world_body->removeCollisionShape(pointers->proxy_shape);
    } else {
        pointers = new pointer_bundle;
        *old_state = pointers;
    }

    pointers->tva = new TriangleVertexArray(vertex_count, vertices, 3 * sizeof(float),
                                            triangle_count, indices, 3 * sizeof(int),
                                            TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
                                            TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    pointers->tri_mesh = new TriangleMesh;
    pointers->tri_mesh->addSubpart(pointers->tva);
    pointers->mesh = new ConcaveMeshShape(pointers->tri_mesh);


    Vector3 chunk_pos(offset[0], offset[1], offset[2]);
    Transform transform(chunk_pos, Quaternion::identity());
    pointers->proxy_shape = world->world_body->addCollisionShape(pointers->mesh, transform, 1);

    return pointers;
}

#ifdef __cplusplus
}
#endif
