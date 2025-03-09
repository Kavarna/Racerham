#pragma once

#include "Jnrlib.h"
#include <glm/glm.hpp>

class Entity;

namespace Components
{

struct Base
{
    glm::mat4x4 world;
    std::string name;

    /* Should never update inverse world manually, it's automatically updated by
     * Entity::UpdateBase */
    glm::mat4x4 inverseWorld;

    Entity *entityPtr;
};

} // namespace Components
