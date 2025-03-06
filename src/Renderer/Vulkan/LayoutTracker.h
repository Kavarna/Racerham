#pragma once

#include "VulkanLoader.h"

namespace Vulkan
{
class Image;

class LayoutTracker
{
public:
    LayoutTracker();
    ~LayoutTracker();

public:
    void TransitionBackBufferImage(u32 index, VkImageLayout newLayout);
    void TransitionImage(Image *img, VkImageLayout newLayout);

    VkImageLayout GetBackbufferImageLayout(u32 index);
    VkImageLayout GetImageLayout(Image *img);

    void Flush();

private:
    std::unordered_map<Image *, VkImageLayout> mImageLayouts;
    std::unordered_map<u32, VkImageLayout> mBackbufferLayouts;
};

} // namespace Vulkan