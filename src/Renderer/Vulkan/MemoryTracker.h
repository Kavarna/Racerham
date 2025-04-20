#pragma once

#include "VulkanLoader.h"
#include <exception>

#include "Buffer.h"
#include "Image.h"

namespace Vulkan
{

class MemoryTracker
{
public:
    MemoryTracker();
    ~MemoryTracker();

    MemoryTracker(MemoryTracker const &) = delete;
    MemoryTracker &operator=(MemoryTracker const &) = delete;

    MemoryTracker(MemoryTracker &&rhs)
    {
        *this = std::move(rhs);
    }
    MemoryTracker &operator=(MemoryTracker &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mBuffers, rhs.mBuffers);
            std::swap(mImages, rhs.mImages);
        }

        return *this;
    }

public:
    void AddBuffer(Buffer &&buffer);
    void AddImage(Image &&image);

    void Flush();

private:
    std::vector<Buffer> mBuffers;
    std::vector<Image> mImages;
};
} // namespace Vulkan
