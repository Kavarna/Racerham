#include "SynchronizationObjects.h"
#include "Renderer.h"
#include "vulkan/vulkan_core.h"

using namespace Vulkan;

GPUSynchronizationObject::GPUSynchronizationObject()
{
    auto device = Renderer::Get()->GetDevice();
    VkSemaphoreCreateInfo semaphoreInfo{};
    {
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;
    }
    vkThrowIfFailed(
        jnrCreateSemaphore(device, &semaphoreInfo, nullptr, &mSemaphore));
}

GPUSynchronizationObject::~GPUSynchronizationObject()
{
    auto device = Renderer::Get()->GetDevice();
    if (mSemaphore != VK_NULL_HANDLE)
    {
        jnrDestroySemaphore(device, mSemaphore, nullptr);
    }
}

CPUSynchronizationObject::CPUSynchronizationObject(bool signaled)
{
    auto device = Renderer::Get()->GetDevice();
    VkFenceCreateInfo fenceInfo{};
    {
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    }
    vkThrowIfFailed(jnrCreateFence(device, &fenceInfo, nullptr, &mFence));
}

CPUSynchronizationObject::~CPUSynchronizationObject()
{
    auto device = Renderer::Get()->GetDevice();
    if (mFence != VK_NULL_HANDLE)
    {

        jnrDestroyFence(device, mFence, nullptr);
    }
}

void CPUSynchronizationObject::Wait()
{
    auto device = Renderer::Get()->GetDevice();
    vkThrowIfFailed(jnrWaitForFences(device, 1, &mFence, VK_TRUE, UINT64_MAX));
}

void CPUSynchronizationObject::WaitAndReset()
{
    Wait();
    Reset();
}

void CPUSynchronizationObject::Reset()
{
    auto device = Renderer::Get()->GetDevice();
    vkThrowIfFailed(jnrResetFences(device, 1, &mFence));
}
