#pragma once

#include "Jnrlib.h"

#include <glm/glm.hpp>

namespace Constants
{
static constexpr u32 MAX_IN_FLIGHT_FRAMES = 3;
constexpr const static glm::vec4 DEFAULT_FORWARD_DIRECTION =
    glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
constexpr const static glm::vec4 DEFAULT_RIGHT_DIRECTION =
    glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
constexpr const static glm::vec4 DEFAULT_UP_DIRECTION =
    glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
} // namespace Constants
