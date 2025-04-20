#include "MemoryTracker.h"

#include "Buffer.h"
#include "Image.h"

using namespace Vulkan;

constexpr const u32 PREALLOCATED_ENTRIES = 32;

MemoryTracker::MemoryTracker()
{
    mBuffers.reserve(PREALLOCATED_ENTRIES);
}

MemoryTracker::~MemoryTracker()
{
}

void MemoryTracker::AddBuffer(Buffer &&buffer)
{
    mBuffers.emplace_back(std::move(buffer));
}

void Vulkan::MemoryTracker::AddImage(Image &&image)
{
    mImages.emplace_back(std::move(image));
}

void Vulkan::MemoryTracker::Flush()
{
    mBuffers.clear();
    mImages.clear();
}
