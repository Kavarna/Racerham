#pragma once

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "entt/entt.hpp"

#include <memory>

#include "Gameplay/Components/Base.h"
#include "Gameplay/Components/Mesh.h"
#include "Gameplay/Components/RigidBody.h"

namespace Systems
{
class Physics
{
public:
    Physics();
    ~Physics();

public:
    void Update(float dt, entt::registry &registry);

public:
    Components::RigidBody CreateRigidBody(Components::Base const &base,
                                          Components::Mesh const &mesh,
                                          float mass);

private:
    std::unique_ptr<btDefaultCollisionConfiguration>
        mDefaultCollisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> mDispatcher;
    std::unique_ptr<btBroadphaseInterface> mBroadphaseInterface;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mWorld;

    std::vector<std::unique_ptr<btRigidBody>> mRigidBodies;
    std::vector<std::unique_ptr<btCollisionShape>> mCollisionShapes;
    std::vector<std::unique_ptr<btDefaultMotionState>> mDefaultMotionStates;
};
} // namespace Systems
