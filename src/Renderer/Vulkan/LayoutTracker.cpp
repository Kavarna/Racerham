#include "LayoutTracker.h"
#include "Renderer.h"

#include "Image.h"

constexpr const u32 PREALLOCATED_ENTRIES = 32;

using namespace Vulkan;

LayoutTracker::LayoutTracker()
{
    mImageLayouts.reserve(PREALLOCATED_ENTRIES);
}

LayoutTracker::~LayoutTracker()
{
}

void LayoutTracker::TransitionBackBufferImage(u32 index,
                                              VkImageLayout newLayout)
{
    mBackbufferLayouts[index] = newLayout;
}

void LayoutTracker::TransitionImage(Image &img, VkImageLayout newLayout)
{
    mImageLayouts[&img] = newLayout;
}

VkImageLayout LayoutTracker::GetBackbufferImageLayout(u32 index)
{
    if (auto it = mBackbufferLayouts.find(index);
        it != mBackbufferLayouts.end())
        return it->second;

    auto renderer = Renderer::Get();
    mBackbufferLayouts[index] = renderer->mSwapchainImageLayouts[index];
    return renderer->mSwapchainImageLayouts[index];
}

VkImageLayout LayoutTracker::GetImageLayout(Image &img)
{
    if (auto it = mImageLayouts.find(&img); it != mImageLayouts.end())
        return it->second;

    mImageLayouts[&img] = img.mLayout;
    return img.mLayout;
}

void LayoutTracker::Flush()
{
    auto renderer = Renderer::Get();
    for (auto &imageLayout : mImageLayouts)
    {
        auto &[image, layout] = imageLayout;
        image->mLayout = layout;
    }
    mImageLayouts.clear();
    for (auto &backbufferLayout : mBackbufferLayouts)
    {
        auto &[index, layout] = backbufferLayout;
        renderer->mSwapchainImageLayouts[index] = layout;
    }
    mBackbufferLayouts.clear();
}
