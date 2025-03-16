#pragma once

#include "Jnrlib.h"

namespace Components
{
struct Indices
{
    u32 firstIndex;
    u32 firstVertex;

    u32 indexCount;
    u32 vertexCount;
};

struct Mesh
{
    std::string path;
    Indices indices;
};
} // namespace Components
