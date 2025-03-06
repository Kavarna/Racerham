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
class Framebuffer;

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

    void CopyBuffer(Vulkan::Buffer *dst, Vulkan::Buffer *src);
    void CopyBuffer(Vulkan::Buffer *dst, u32 dstOffset, Vulkan::Buffer *src);
    void CopyBuffer(Vulkan::Buffer *dst, u32 dstOffset, Vulkan::Buffer *src, u32 srcOffset);

    void BindVertexBuffer(Vulkan::Buffer const *buffer, u32 firstIndex);
    void BindIndexBuffer(Vulkan::Buffer const *buffer);

    void BindPipeline(Pipeline *pipeline);
    void BindDescriptorSet(DescriptorSet *set, u32 descriptorSetInstance, RootSignature *rootSignature);
    void SetScissor(std::vector<VkRect2D> const &scissors);
    void SetViewports(std::vector<VkViewport> const &viewports);
    void Draw(u32 vertexCount, u32 firstVertex);
    void DrawIndexedInstanced(u32 indexCount, u32 firstIndex, u32 vertexOffset);

    void TransitionBackbufferTo(TransitionInfo const &transitionInfo);
    void TransitionImageTo(Image *img, TransitionInfo const &transitionInfo);

    void CopyWholeBufferToImage(Image *, Buffer *);

    void BeginRenderPassOnBackbuffer(RenderPass *rp, std::vector<Framebuffer *> const &fb, float const *clearColor);
    void EndRenderPass();

    void BeginRenderingOnBackbuffer(float const backgroundColor[4]);
    void BeginRenderingOnImage(Image *img, float const backgroundColor[4], Image *depth, bool useStencil);
    void EndRendering();

    void AddLocalBuffer(std::unique_ptr<Buffer> &&buffer);
    void AddLocalImage(std::unique_ptr<Image> &&buffer);

    void Submit(CPUSynchronizationObject *signalWhenFinished);
    void SubmitToScreen(CPUSynchronizationObject *signalWhenFinished = nullptr);
    void SubmitAndWait();

public:
    template <typename T>
    void BindPushRange(RootSignature *rootSignature, u32 offset, u32 count, T const *data,
                       VkShaderStageFlags stages = VK_SHADER_STAGE_ALL)
    {
        jnrCmdPushConstants(mCommandBuffers[mActiveCommandIndex], rootSignature->mPipelineLayout, stages, offset,
                            sizeof(T) * count, data);
    }

private:
    void *CopyToTemporaryStorage(u8 *data, u8 dataSize);

private:
    VkCommandPool mCommandPool;
    CommandListType mType;

    u32 mActiveCommandIndex = 0;

    std::vector<VkCommandBuffer> mCommandBuffers;

    LayoutTracker mLayoutTracker;
    MemoryTracker mMemoryTracker;

    /* TODO: Maybe do something smarter once multiple synchronization objects will be needed */
    std::unique_ptr<GPUSynchronizationObject> mBackbufferAvailable = nullptr;
    std::unique_ptr<GPUSynchronizationObject> mRenderingFinished = nullptr;

    /* Current recording info */
    i32 mImageIndex;
    u8 mTemporaryStorage[TEMPORARY_STORAGE_SIZE];
    u8 mTemporaryStorageOffset;
};
} // namespace Vulkan