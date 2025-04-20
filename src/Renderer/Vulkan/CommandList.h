#pragma once

#include "Buffer.h"
#include "LayoutTracker.h"
#include "MemoryTracker.h"
#include "RootSignature.h"
#include "SynchronizationObjects.h"
#include <Jnrlib.h>

namespace Vulkan
{
class Pipeline;
class DescriptorSet;
class Renderer;
class RenderPass;
#if USE_RENDERPASS
class Framebuffer;
#endif

enum class CommandListType
{
    Graphics = 0,
};

class CommandList
{
    static constexpr const u32 TEMPORARY_STORAGE_SIZE = 512;

public:
    CommandList(CommandListType cmdListType);
    ~CommandList();

    CommandList(const CommandList &) = delete;
    CommandList(CommandList &&rhs)
    {
        *this = std::move(rhs);
    }
    CommandList &operator=(const CommandList &) = delete;
    CommandList &operator=(CommandList &&rhs)
    {
        if (this != &rhs)
        {
            std::swap(mCommandPool, rhs.mCommandPool);
            std::swap(mType, rhs.mType);
            std::swap(mActiveCommandIndex, rhs.mActiveCommandIndex);
            std::swap(mCommandBuffers, rhs.mCommandBuffers);
            std::swap(mLayoutTracker, rhs.mLayoutTracker);
            std::swap(mMemoryTracker, rhs.mMemoryTracker);
            std::swap(mBackbufferAvailableSyncIndex,
                      rhs.mBackbufferAvailableSyncIndex);
            std::swap(mRenderingFinishedSyncIndex,
                      rhs.mRenderingFinishedSyncIndex);
            std::swap(mGPUSynchronizationObjects,
                      rhs.mGPUSynchronizationObjects);
            std::swap(mImageIndex, rhs.mImageIndex);
        }

        return *this;
    }

public:
    struct TransitionInfo
    {
        VkImageLayout newLayout;
        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
    };

public:
    void Init(u32 numCommandBuffers = 1);
    void ResetAll();

    u32 GetCurrentBackbufferIndex();

    void Begin();
    void End();

    void CopyBuffer(Vulkan::Buffer &dst, Vulkan::Buffer const &src);
    void CopyBuffer(Vulkan::Buffer &dst, u32 dstOffset,
                    Vulkan::Buffer const &src);
    void CopyBuffer(Vulkan::Buffer &dst, u32 dstOffset,
                    Vulkan::Buffer const &src, u32 srcOffset);

    void BindVertexBuffer(Vulkan::Buffer const &buffer, u32 firstIndex);
    void BindIndexBuffer(Vulkan::Buffer const &buffer);

    void BindPipeline(Pipeline &pipeline);
    void BindDescriptorSet(DescriptorSet &set, u32 descriptorSetInstance,
                           RootSignature &rootSignature);
    void SetScissor(std::vector<VkRect2D> const &scissors);
    void SetViewports(std::vector<VkViewport> const &viewports);
    void Draw(u32 vertexCount, u32 firstVertex);
    void DrawIndexedInstanced(u32 indexCount, u32 firstIndex, u32 vertexOffset);

    void TransitionBackbufferTo(TransitionInfo const &transitionInfo);
    void TransitionImageTo(Image *img, TransitionInfo const &transitionInfo);

    void CopyWholeBufferToImage(Image *, Buffer *);

#if USE_RENDERPASS
    void BeginRenderPassOnBackbuffer(RenderPass *rp,
                                     std::vector<Framebuffer *> const &fb,
                                     float const *clearColor);
    void EndRenderPass();

#endif /* USE_RENDERPASS */

    void BeginRenderingOnBackbuffer(float const backgroundColor[4],
                                    Image *depth, bool useStencil);
    void BeginRenderingOnImage(Image *img, float const backgroundColor[4],
                               Image *depth, bool useStencil);
    void EndRendering();

    void AddLocalBuffer(Buffer &&buffer);
    void AddLocalImage(Image &&buffer);

    void Submit(CPUSynchronizationObject const &signalWhenFinished);
    void SubmitToScreen(CPUSynchronizationObject const &signalWhenFinished);
    void SubmitAndWait();

public:
    template <typename T>
    void BindPushRange(RootSignature &rootSignature, u32 offset, u32 count,
                       T const *data,
                       VkShaderStageFlags stages = VK_SHADER_STAGE_ALL)
    {
        jnrCmdPushConstants(mCommandBuffers[mActiveCommandIndex],
                            rootSignature.mPipelineLayout, stages, offset,
                            sizeof(T) * count, data);
    }

private:
    u32 GetNewSyncObjectIndex()
    {
        u32 index = (u32)mGPUSynchronizationObjects.size();

        mGPUSynchronizationObjects.emplace_back();

        return index;
    }

private:
    VkCommandPool mCommandPool;
    CommandListType mType;

    u32 mActiveCommandIndex = 0;

    std::vector<VkCommandBuffer> mCommandBuffers;

    LayoutTracker mLayoutTracker;
    MemoryTracker mMemoryTracker;

    u32 mBackbufferAvailableSyncIndex = -1;
    u32 mRenderingFinishedSyncIndex = -1;
    std::vector<GPUSynchronizationObject> mGPUSynchronizationObjects;

    /* Current recording info */
    i32 mImageIndex;
};
} // namespace Vulkan
