#include "Physics.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "Check.h"
#include "Gameplay/Components/RigidBody.h"
#include "LinearMath/btDefaultMotionState.h"
#include "LinearMath/btTransform.h"
#include "entt/entity/fwd.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Systems
{
Physics::Physics()
{
    mDefaultCollisionConfiguration =
        std::make_unique<btDefaultCollisionConfiguration>();
    mDispatcher = std::make_unique<btCollisionDispatcher>(
        mDefaultCollisionConfiguration.get());
    mBroadphaseInterface = std::make_unique<btDbvtBroadphase>();
    mSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
    mWorld = std::make_unique<btDiscreteDynamicsWorld>(
        mDispatcher.get(), mBroadphaseInterface.get(), mSolver.get(),
        mDefaultCollisionConfiguration.get());

    mWorld->setGravity(btVector3(0.0f, -10.0f, 0.0f));
}

Physics::~Physics()
{
    mWorld.reset();
    mDispatcher.reset();
}

Components::RigidBody Physics::CreateRigidBody(Components::Base const &base,
                                               Components::Mesh const &mesh,
                                               float mass)
{
    CHECK_FATAL(
        mesh.path == "cube",
        "As of right now only cubes can be used to create rigid bodies");

    std::unique_ptr<btCollisionShape> collisionShape =
        std::make_unique<btBoxShape>(btVector3(1.0f, 1.0f, 1.0f));

    btTransform startTransform;
    if (false)
    {
        /* TODO: When physics debugging is available, check why this doesn't
         * work */
        const float *worldMatrix = glm::value_ptr(base.world);
        startTransform.setFromOpenGLMatrix(worldMatrix);
    }

    btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
    if (mass != 0.0f)
    {
        collisionShape->calculateLocalInertia(mass, localInertia);
    }

    {
        glm::vec3 scale, translation, skew;
        glm::quat rotation;
        glm::vec4 perspective;

        CHECK_FATAL(glm::decompose(base.world, scale, rotation, translation,
                                   skew, perspective),
                    "Could not decompose matrix");

        startTransform.setIdentity();
        startTransform.setOrigin(
            btVector3(translation.x, translation.y, translation.z));
        collisionShape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
    }

    std::unique_ptr<btDefaultMotionState> motionState =
        std::make_unique<btDefaultMotionState>(startTransform);

    btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(
        mass, motionState.get(), collisionShape.get(), localInertia);
    std::unique_ptr<btRigidBody> rigidBody =
        std::make_unique<btRigidBody>(rigidBodyInfo);

    mWorld->addRigidBody(rigidBody.get());

    Components::RigidBody result;
    {
        result.rigidBody = rigidBody.get();
        result.collisionShape = collisionShape.get();
        result.motionState = motionState.get();
        result.mass = mass;
    }

    mRigidBodies.emplace_back(std::move(rigidBody));
    mCollisionShapes.emplace_back(std::move(collisionShape));
    mDefaultMotionStates.emplace_back(std::move(motionState));

    return result;
}

void Physics::Update(float dt, entt::registry &registry)
{
    mWorld->stepSimulation(dt);
    if (mWorld->getDebugDrawer())
    {
        mWorld->debugDrawWorld();
    }

    auto rigidBodies = registry.view<Components::Base, Components::RigidBody>();
    for (auto [entity, base, rigidBody] : rigidBodies.each())
    {
        btTransform transform;
        rigidBody.rigidBody->getMotionState()->getWorldTransform(transform);
        if (rigidBody.rigidBody->isActive())
        {

            float floatWorld[16];
            transform.getOpenGLMatrix(floatWorld);

            registry.patch<Components::Base>(
                entity, [&](Components::Base &base) {
                    base.world = glm::make_mat4(floatWorld);
                });
        }
    }
}
} // namespace Systems
