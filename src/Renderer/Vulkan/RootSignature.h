#pragma once

#include "Buffer.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan
{
class Image;
class ImageView;

class DescriptorSet
{
    friend class RootSignature;
    friend class CommandList;

public:
    DescriptorSet();
    ~DescriptorSet();

    /* Make this non-copyable */
    DescriptorSet(const DescriptorSet &) = delete;
    DescriptorSet &operator=(const DescriptorSet &) = delete;

    void AddSampler(u32 binding, std::vector<VkSampler> const &samplers,
                    VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);

    void AddCombinedImageSampler(u32 binding, VkSampler *sampler, VkShaderStageFlags stages);
    void BindCombinedImageSampler(u32 binding, Vulkan::Image *image, VkImageAspectFlags aspectFlags, VkSampler sampler,
                                  u32 instance = 0);
    void BindCombinedImageSampler(u32 binding, Vulkan::ImageView image, VkImageAspectFlags aspectFlags,
                                  VkSampler sampler, u32 instance = 0);

    void AddStorageBuffer(u32 binding, u32 descriptorCount, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
    void BindStorageBuffer(Vulkan::Buffer *buffer, u32 binding, u32 elementIndex = 0, u32 instance = 0);

    void AddInputBuffer(u32 binding, u32 descriptorCount, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
    void BindInputBuffer(Vulkan::Buffer *buffer, u32 binding, u32 elementIndex = 0, u32 instance = 0);

    void BakeLayout();
    /* Bake multiple instances */
    void Bake(u32 instances = 1);

private:
    VkDescriptorSetLayoutCreateInfo mLayoutInfo{};
    VkDescriptorPoolCreateInfo mPoolInfo{};

    u32 mInputBufferCount = 0;
    u32 mStorageBufferCount = 0;
    u32 mSamplerCount = 0;
    u32 mCombinedImageSamplerCount = 0;

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

    /* TODO: Should make this non-copyable */

public:
    template <typename T> void AddPushRange(u32 offset, u32 count = 1, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
    void AddDescriptorSet(DescriptorSet *descriptorSet);

    void Bake();

private:
    std::vector<VkPushConstantRange> mPushRanges;
    std::vector<DescriptorSet *> mDescriptorSetLayouts;

    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};

} // namespace Vulkan

template <typename T> inline void Vulkan::RootSignature::AddPushRange(u32 offset, u32 count, VkShaderStageFlags stages)
{
    VkPushConstantRange pushRange{};
    {
        pushRange.offset = offset;
        pushRange.size = count * sizeof(T);
        pushRange.stageFlags = stages;
    }
    mPushRanges.push_back(pushRange);
}
