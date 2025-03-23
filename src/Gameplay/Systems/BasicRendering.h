#pragma once

#include "Gameplay/Camera.h"
#include "Renderer/Vulkan/Buffer.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/Pipeline.h"
#include "Renderer/Vulkan/RootSignature.h"
#include "Utils/Constants.h"
#include "entt/entt.hpp"

namespace Systems
{
namespace BasicRendering
{

class RenderSystem
{
public:
    RenderSystem();

    RenderSystem(const RenderSystem &) = delete;
    RenderSystem(RenderSystem &&) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;
    RenderSystem &operator=(RenderSystem &&) = delete;

public:
    void OnResize();
    /**
     * @brief Renders all the entities in registry. The output images must have
     * been set before calling this.
     */
    void Render(Vulkan::CommandList *cmdList, u32 currentFrameIndex,
                entt::registry const &registry, u32 objectCount);
    void UpdateCamera(Camera const &camera);

public:
    void SetRenderingBuffers(Vulkan::Buffer *vertexBuffer,
                             Vulkan::Buffer *indexBuffer)
    {
        mVertexBuffer = vertexBuffer;
        mIndexBuffer = indexBuffer;
    }

private:
    void StateInit();
    void ResizeWorldBufferIfNeeded(u32 objectCount);

private:
    std::unique_ptr<Vulkan::Pipeline> mPipeline;
    std::unique_ptr<Vulkan::RootSignature> mRootSignature;
    std::unique_ptr<Vulkan::DescriptorSet> mDescriptorSet;

    Vulkan::Buffer *mVertexBuffer = nullptr;
    Vulkan::Buffer *mIndexBuffer = nullptr;

    std::unique_ptr<Vulkan::Buffer> mWorldBuffer;
    std::unique_ptr<Vulkan::Buffer> mPerFrameBuffer;

    bool mIsDirty = true;
};

} // namespace BasicRendering
} // namespace Systems
