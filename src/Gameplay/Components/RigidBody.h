#pragma once

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btDefaultMotionState.h"
#include <memory>

namespace Components
{
struct RigidBody
{
    btRigidBody *rigidBody;

    btCollisionShape *collisionShape;

    btDefaultMotionState *motionState;

    float mass;
};
} // namespace Components
