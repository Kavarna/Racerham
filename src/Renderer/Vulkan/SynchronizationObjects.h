#pragma once

#include "VulkanLoader.h"
#include "vulkan/vulkan_core.h"

namespace Vulkan
{
class GPUSynchronizationObject
{
public:
    GPUSynchronizationObject();
    ~GPUSynchronizationObject();

    GPUSynchronizationObject(const GPUSynchronizationObject &) = delete;
    GPUSynchronizationObject &operator=(const GPUSynchronizationObject &) =
        delete;

    GPUSynchronizationObject(GPUSynchronizationObject &&rhs)
    {
        *this = std::move(rhs);
    }

    GPUSynchronizationObject &operator=(GPUSynchronizationObject &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mSemaphore, rhs.mSemaphore);
        }
        return *this;
    }

    operator VkSemaphore() const
    {
        return mSemaphore;
    }

private:
    VkSemaphore mSemaphore = VK_NULL_HANDLE;
};

class CPUSynchronizationObject
{
public:
    CPUSynchronizationObject(bool signaled = false);
    ~CPUSynchronizationObject();

    CPUSynchronizationObject(const CPUSynchronizationObject &) = delete;
    CPUSynchronizationObject &operator=(const CPUSynchronizationObject &) =
        delete;

    CPUSynchronizationObject(CPUSynchronizationObject &&rhs)
    {
        *this = std::move(rhs);
    }

    CPUSynchronizationObject &operator=(CPUSynchronizationObject &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mFence, rhs.mFence);
        }
        return *this;
    }

    operator VkFence() const
    {
        return mFence;
    }

public:
    /// <summary>
    /// Wait for this fence to finish. Note: If batches are needed, this
    /// function should not be used
    /// </summary>
    void Wait();

    void WaitAndReset();

    void Reset();

private:
    VkFence mFence = VK_NULL_HANDLE;
};

} // namespace Vulkan
