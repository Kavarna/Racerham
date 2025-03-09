#include "Entity.h"
#include "Components/Base.h"
#include "Components/Update.h"

#include "../Utils/Constants.h"

void Entity::UpdateBase()
{
    auto &u = GetComponent<Components::Update>();
    u.dirtyFrames = Constants::MAX_IN_FLIGHT_FRAMES;

    auto &base = GetComponent<Components::Base>();
    base.inverseWorld = glm::inverse(base.world);
}

void Entity::SetParent(Entity *parentEntity)
{
    if (parentEntity == nullptr)
        return;

    if (parentEntity == mParentEntity)
        return;

    mParentEntity = parentEntity;

    parentEntity->AddChild(this);
}

void Entity::AddChild(Entity *childEntity)
{
    if (childEntity->mParentEntity == this)
        return;

    for (uint32_t i = 0; i < mChildEntities.size(); ++i)
    {
        if (mChildEntities[i] == childEntity)
        {
            return;
        }
    }

    childEntity->mParentEntity = this;
    AddChildUnsafe(childEntity);
}

uint32_t Entity::GetEntityId() const
{
    return (size_t)mEntity;
}

bool Entity::HasChildren() const
{
    return mChildEntities.size();
}

std::vector<Entity *> const &Entity::GetChildren() const
{
    return mChildEntities;
}

void Entity::AddChildUnsafe(Entity *childEntity)
{
    mChildEntities.push_back(childEntity);
}
