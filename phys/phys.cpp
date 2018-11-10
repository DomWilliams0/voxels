#include "phys.hpp"
#include "reactphysics3d.h"

#include <iostream>

using namespace reactphysics3d;

extern "C" void do_thing() {
    Vector3 gravity(0, -9.81f, 0);
    DynamicsWorld world(gravity);
}
