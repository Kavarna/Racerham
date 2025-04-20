#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan
{
class RootSignature;
class RenderPass;

class Pipeline
{
    friend class CommandList;

public:
    Pipeline(std::string const &name) : mName(name)
    {
        InitDefaultPipelineState();
    }
    ~Pipeline()
    {
        Clear();
    }

    Pipeline(Pipeline const &) = delete;
    Pipeline &operator=(Pipeline const &) = delete;

    Pipeline(Pipeline &&rhs)
    {
        *this = std::move(rhs);
    }
    Pipeline &operator=(Pipeline &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mName, rhs.mName);
            std::swap(mRootSignature, rhs.mRootSignature);
            std::swap(mColorOutputs, rhs.mColorOutputs);
            std::swap(mDepthFormat, rhs.mDepthFormat);
            std::swap(mStencilFormat, rhs.mStencilFormat);
            std::swap(mShaderModules, rhs.mShaderModules);
            std::swap(mBlendState, rhs.mBlendState);
            std::swap(mDepthStencilState, rhs.mDepthStencilState);
            std::swap(mDynamicState, rhs.mDynamicState);
            std::swap(mInputAssemblyState, rhs.mInputAssemblyState);
            std::swap(mMultisampleState, rhs.mMultisampleState);
            std::swap(mRasterizationState, rhs.mRasterizationState);
            std::swap(mTesselationState, rhs.mTesselationState);
            std::swap(mVertexInputState, rhs.mVertexInputState);
            std::swap(mViewportState, rhs.mViewportState);
            std::swap(mRenderingInfo, rhs.mRenderingInfo);
            std::swap(mPipelineInfo, rhs.mPipelineInfo);
            std::swap(mPipeline, rhs.mPipeline);
        }

        return *this;
    }

public:
    void Clear(bool reinit = false);
    void AddShader(std::string const &path);
    void AddShader(std::vector<char> const &shaderContent,
                   VkShaderStageFlags shaderStage);

    void SetRootSignature(RootSignature const *rootSignature);

    void AddBackbufferColorOutput();
    void AddImageColorOutput(class Image const *img);
    void AddFormatColorOutput(VkFormat format);
    void SetBackbufferDepthStencilOutput();
    void SetDepthImage(class Image const *img);
    void SetDepthStencilImage(class Image const *img);

    void InitFrom(Pipeline const &p);

    void Bake(
#if USE_RENDERPASS
        RenderPass * = nullptr
#endif /* USE_RENDERPASS */
    );

public:
    VkPipelineColorBlendStateCreateInfo &GetColorBlendStateCreateInfo()
    {
        return mBlendState;
    }
    VkPipelineDepthStencilStateCreateInfo &GetDepthStencilStateCreateInfo()
    {
        return mDepthStencilState;
    }
    VkPipelineDynamicStateCreateInfo &GetDynamicStateCreateInfo()
    {
        return mDynamicState;
    }
    VkPipelineInputAssemblyStateCreateInfo &GetInputAssemblyStateCreateInfo()
    {
        return mInputAssemblyState;
    }
    VkPipelineMultisampleStateCreateInfo &GetMultisampleStateCreateInfo()
    {
        return mMultisampleState;
    }
    VkPipelineRasterizationStateCreateInfo &GetRasterizationStateCreateInfo()
    {
        return mRasterizationState;
    }
    VkPipelineTessellationStateCreateInfo &GetTessellationStateCreateInfo()
    {
        return mTesselationState;
    }
    VkPipelineVertexInputStateCreateInfo &GetVertexInputStateCreateInfo()
    {
        return mVertexInputState;
    }
    VkPipelineViewportStateCreateInfo &GetViewportStateCreateInfo()
    {
        return mViewportState;
    }

private:
    void InitDefaultPipelineState();

private:
    std::string mName = "";

    RootSignature const *mRootSignature = nullptr;

    std::vector<VkFormat> mColorOutputs;
    VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
    VkFormat mStencilFormat = VK_FORMAT_UNDEFINED;
    std::vector<VkPipelineShaderStageCreateInfo> mShaderModules;

    VkPipelineColorBlendStateCreateInfo mBlendState{};
    VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};
    VkPipelineDynamicStateCreateInfo mDynamicState{};
    VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState{};
    VkPipelineMultisampleStateCreateInfo mMultisampleState{};
    VkPipelineRasterizationStateCreateInfo mRasterizationState{};
    VkPipelineTessellationStateCreateInfo mTesselationState{};
    VkPipelineVertexInputStateCreateInfo mVertexInputState{};
    VkPipelineViewportStateCreateInfo mViewportState{};

    VkPipelineRenderingCreateInfo mRenderingInfo{};
    VkGraphicsPipelineCreateInfo mPipelineInfo{};

    VkPipeline mPipeline = VK_NULL_HANDLE;
};

} // namespace Vulkan