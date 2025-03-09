#pragma once

#include "Singletone.h"
#include "entt/entity/fwd.hpp"
#include "src/Gameplay/Entity.h"
class World : public Jnrlib::ISingletone<World>
{
    MAKE_SINGLETONE_CAPABLE(World);

private:
    World();
    ~World();

public:
private:
    entt::registry mRegistry;

    std::vector<std::unique_ptr<Entity>> mEntities;
};
