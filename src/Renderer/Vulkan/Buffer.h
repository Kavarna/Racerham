#pragma once

#include "Renderer.h"
#include "VulkanLoader.h"
#include "vulkan/vulkan_core.h"

#include <cstring>

namespace Vulkan
{
class Buffer
{
    friend class CommandList;
    friend class DescriptorSet;

public:
    Buffer() = default;

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&rhs)
    {
        *this = std::move(rhs);
    }

    Buffer &operator=(Buffer &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mBuffer, rhs.mBuffer);
            std::swap(mAllocation, rhs.mAllocation);
            std::swap(mAllocationInfo, rhs.mAllocationInfo);
            std::swap(mElementSize, rhs.mElementSize);
            std::swap(mCount, rhs.mCount);
            std::swap(mData, rhs.mData);
        }
        return *this;
    }

    Buffer(u64 elementSize, u64 count, VkBufferUsageFlags usage,
           VmaAllocationCreateFlags allocationFlags = 0)
        : mElementSize(elementSize), mCount(count)
    {
        bool mappable =
            ((allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) !=
                 0 ||
             (allocationFlags &
              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0);

        auto allocator = Renderer::Get()->GetAllocator();

        VkBufferCreateInfo bufferInfo{};
        {
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.queueFamilyIndexCount =
                1; /* TODO: If needed improve this */
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.size = elementSize * count;
            bufferInfo.usage = usage;
        }

        VmaAllocationCreateInfo allocationInfo{};
        {
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocationInfo.flags = allocationFlags;
        }

        vkThrowIfFailed(vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo,
                                        &mBuffer, &mAllocation,
                                        &mAllocationInfo));

        if (mappable)
        {
            auto allocator = Renderer::Get()->GetAllocator();

            vmaMapMemory(allocator, mAllocation, (void **)&mData);
        }
    }

    void Copy(void *src)
    {
        ThrowIfFailed(mData != nullptr,
                      "In order to copy into a buffer, it must be mappable");
        memcpy(mData, src, mElementSize * mCount);
    }

    void *GetElement(u32 index = 0)
    {
        ThrowIfFailed(mData != nullptr,
                      "In order to get the address of an element inside the "
                      "buffer, it must be mappable");
        return (unsigned char *)mData + mElementSize * index;
    }

    void const *GetElement(u32 index = 0) const
    {
        ThrowIfFailed(mData != nullptr,
                      "In order to get the address of an element inside the "
                      "buffer, it must be mappable");
        return (unsigned char *)mData + mElementSize * index;
    }

    void *GetData()
    {
        ThrowIfFailed(mData != nullptr,
                      "In order to get the address of an element inside the "
                      "buffer, it must be mappable");
        return mData;
    }

    u64 GetElementSize() const
    {
        return mElementSize;
    }

    u64 GetSize() const
    {
        return mElementSize * mCount;
    }

    u64 GetCount() const
    {
        return mCount;
    }

    ~Buffer()
    {
        auto allocator = Renderer::Get()->GetAllocator();

        if (mData != nullptr)
        {
            auto allocator = Renderer::Get()->GetAllocator();
            vmaUnmapMemory(allocator, mAllocation);
        }

        if (mBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, mBuffer, mAllocation);
        }
    }

private:
    VkBuffer mBuffer = VK_NULL_HANDLE;
    VmaAllocation mAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo mAllocationInfo = {};

    u64 mElementSize = 0;
    u64 mCount = 0;
    void *mData = nullptr;
};

} // namespace Vulkan
