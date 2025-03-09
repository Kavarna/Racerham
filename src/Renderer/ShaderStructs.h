#pragma once

#include <Jnrlib.h>
#include <glm/glm.hpp>

struct BasicPerObjectInfo
{
    glm::mat4 world;
};

struct BasicPushConstant
{
    u32 objectIndex;
};
