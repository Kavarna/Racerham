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

void MemoryTracker::AddBuffer(std::unique_ptr<Buffer> &&buffer)
{
    ThrowIfFailed(buffer != nullptr, "Can not register an null-buffer in the memory tracker");
    mBuffers.emplace_back(std::move(buffer));
}

void Vulkan::MemoryTracker::AddImage(std::unique_ptr<Image> &&image)
{
    ThrowIfFailed(image != nullptr, "Can not register an null-buffer in the memory tracker");
    mImages.emplace_back(std::move(image));
}

void Vulkan::MemoryTracker::Flush()
{
    for (auto &buffer : mBuffers)
    {
        buffer.reset();
    }
    mBuffers.clear();

    for (auto &image : mImages)
    {
        image.reset();
    }
    mImages.clear();
}
