#pragma once

#include "Buffer.h"
#include "Image.h"

#include "Check.h"
#include "vulkan/vulkan_core.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan
{

class DescriptorSet
{
    friend class RootSignature;
    friend class CommandList;

public:
    DescriptorSet() = default;
    ~DescriptorSet();

    DescriptorSet(const DescriptorSet &) = delete;
    DescriptorSet &operator=(const DescriptorSet &) = delete;

    DescriptorSet(DescriptorSet &&rhs)
    {
        *this = std::move(rhs);
    }
    DescriptorSet &operator=(DescriptorSet &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mLayoutInfo, rhs.mLayoutInfo);
            std::swap(mPoolInfo, rhs.mPoolInfo);
            std::swap(mInputBufferCount, rhs.mInputBufferCount);
            std::swap(mStorageBufferCount, rhs.mStorageBufferCount);
            std::swap(mSamplerCount, rhs.mSamplerCount);
            std::swap(mCombinedImageSamplerCount,
                      rhs.mCombinedImageSamplerCount);
            std::swap(mActiveInstance, rhs.mActiveInstance);
            std::swap(mBindings, rhs.mBindings);
            std::swap(mLayout, rhs.mLayout);
            std::swap(mDescriptorPool, rhs.mDescriptorPool);
            std::swap(mDescriptorSets, rhs.mDescriptorSets);
        }

        return *this;
    }

    void AddSampler(u32 binding, std::vector<VkSampler> const &samplers,
                    VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);

    void AddCombinedImageSampler(
        u32 binding, VkSampler *sampler,
        VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
    void BindCombinedImageSampler(u32 binding, Vulkan::Image &image,
                                  VkImageAspectFlags aspectFlags,
                                  VkSampler sampler);
    void BindCombinedImageSampler(u32 binding, Vulkan::ImageView image,
                                  VkImageAspectFlags aspectFlags,
                                  VkSampler sampler);

    void AddStorageBuffer(u32 binding, u32 descriptorCount,
                          VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
    void BindStorageBuffer(Vulkan::Buffer &buffer, u32 binding,
                           u32 elementIndex = 0);

    void AddInputBuffer(u32 binding, u32 descriptorCount,
                        VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
    void BindInputBuffer(Vulkan::Buffer &buffer, u32 binding,
                         u32 elementIndex = 0);

    void BakeLayout();
    /* Bake multiple instances */
    void Bake(u32 instances = 1);

    void SetActiveInstance(u32 activeInstance)
    {
        CHECK_FATAL(activeInstance < mPoolInfo.maxSets, activeInstance,
                    " is bigger than ", mPoolInfo.maxSets,
                    " which is the maximum number of instances");

        mActiveInstance = activeInstance;
    }

private:
    VkDescriptorSetLayoutCreateInfo mLayoutInfo{};
    VkDescriptorPoolCreateInfo mPoolInfo{};

    u32 mInputBufferCount = 0;
    u32 mStorageBufferCount = 0;
    u32 mSamplerCount = 0;
    u32 mCombinedImageSamplerCount = 0;

    u32 mActiveInstance = 0;

    std::vector<VkDescriptorSetLayoutBinding> mBindings;
    VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;

    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> mDescriptorSets;
};

class RootSignature
{
    friend class Pipeline;
    friend class CommandList;

public:
    RootSignature();
    ~RootSignature();

    RootSignature(RootSignature const &) = delete;
    RootSignature &operator=(RootSignature const &) = delete;

    RootSignature(RootSignature &&rhs)
    {
        *this = std::move(rhs);
    }

    RootSignature &operator=(RootSignature &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mPushRanges, rhs.mPushRanges);
            std::swap(mDescriptorSetLayouts, rhs.mDescriptorSetLayouts);
            std::swap(mPipelineLayout, rhs.mPipelineLayout);
        }

        return *this;
    }

    /* TODO: Should make this non-copyable */

public:
    template <typename T>
    void AddPushRange(u32 offset, u32 count = 1,
                      VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
    void AddDescriptorSet(DescriptorSet *descriptorSet);

    void Bake();

private:
    std::vector<VkPushConstantRange> mPushRanges;
    std::vector<DescriptorSet *> mDescriptorSetLayouts;

    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};

} // namespace Vulkan

template <typename T>
inline void Vulkan::RootSignature::AddPushRange(u32 offset, u32 count,
                                                VkShaderStageFlags stages)
{
    VkPushConstantRange pushRange{};
    {
        pushRange.offset = offset;
        pushRange.size = count * sizeof(T);
        pushRange.stageFlags = stages;
    }
    mPushRanges.push_back(pushRange);
}
