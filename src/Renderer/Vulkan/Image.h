#pragma once

#include "MemoryAllocator.h"
#include "VulkanLoader.h"

namespace Vulkan
{
class ImageView
{
public:
    ImageView(VkImageView imageView, VkImageAspectFlags aspect, VkImageLayout mLayout)
        : mImageView(imageView), mAspectFlags(aspect)
    {
    }

    VkImageView GetView() const
    {
        return mImageView;
    }

    VkImageAspectFlags GetAspect() const
    {
        return mAspectFlags;
    }

    VkImageLayout GetLayout() const
    {
        return mLayout;
    }

    void SetImageView(VkImageView imageView)
    {
        mImageView = imageView;
    }

    void SetAspect(VkImageAspectFlags aspectFlags)
    {
        mAspectFlags = aspectFlags;
    }

    void SetLayout(VkImageLayout layout)
    {
        mLayout = layout;
    }

private:
    VkImageView mImageView;
    VkImageAspectFlags mAspectFlags;
    VkImageLayout mLayout;
};

class Image
{
    friend class LayoutTracker;
    friend class CommandList;

public:
    struct Info2D
    {
        u32 width, height;
        VkImageUsageFlags usage;
        VkFormat format;
        VkImageLayout initialLayout;

        VmaAllocationCreateFlags allocationFlags = 0;
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO;
        std::vector<u32> queueFamilies{};
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    };

public:
    Image(Info2D const &info);
    ~Image();

    void EnsureAspect(VkImageAspectFlags aspectMask);
    ImageView GetImageView(VkImageAspectFlags aspectMask);
    VkExtent2D GetExtent2D() const;
    VkImageLayout GetLayout() const;
    VkImageUsageFlags GetUsage() const;
    VkSampleCountFlagBits GetSampleCount() const;

    VkFormat GetFormat() const;

public:
    void SetPixelColor(u32 x, u32 y, float color[4]);

private:
    VkImageCreateInfo mCreateInfo{};
    VkImage mImage;

    /* Pretty much used only to be working with the default back-end */
    VkDescriptorSet mImguiTextureID = VK_NULL_HANDLE;

    std::unordered_map<VkImageAspectFlags, VkImageView> mImageViews;
    VkImageLayout mLayout;

    std::vector<u32> mQueueFamilies;
    VkFormat mFormat;
    VkImageType mImageType;
    VkExtent2D mExtent2D;

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    bool mMappable;
    void *mData = nullptr;
};

} // namespace Vulkan