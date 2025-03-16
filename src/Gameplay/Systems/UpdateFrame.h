#pragma once

#include <entt/entt.hpp>

#include "Gameplay/Components/Update.h"

namespace Systems
{
class UpdateFrame
{
public:
    void Update(entt::registry &registry)
    {
        auto updatables = registry.view<Components::Update>();
        for (auto [entity, update] : updatables.each())
        {
            if (update.dirtyFrames)
            {
                update.dirtyFrames--;
            }
        }
    }
};
} // namespace Systems
