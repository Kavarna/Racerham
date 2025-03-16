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

struct SharedState
{
    friend class RenderSystem;

public:
    SharedState(u32 numInstances = Constants::MAX_IN_FLIGHT_FRAMES);

    SharedState(const SharedState &) = delete;
    SharedState(SharedState &&) = delete;
    SharedState &operator=(const SharedState &) = delete;
    SharedState &operator=(SharedState &&) = delete;

private:
    std::unique_ptr<Vulkan::Pipeline> mPipeline;
    std::unique_ptr<Vulkan::RootSignature> mRootSignature;
    std::unique_ptr<Vulkan::DescriptorSet> mDescriptorSet;
};

class RenderSystem
{
public:
    RenderSystem(SharedState *sharedState, u32 sharedStateIndex);

    RenderSystem(const RenderSystem &) = delete;
    RenderSystem(RenderSystem &&) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;
    RenderSystem &operator=(RenderSystem &&) = delete;

public:
    void Render(Vulkan::CommandList *cmdList, entt::registry const &registry,
                u32 objectCount);
    void UpdateCamera(Camera const &camera);

public:
    void SetRenderingBuffers(Vulkan::Buffer *vertexBuffer,
                             Vulkan::Buffer *indexBuffer)
    {
        mVertexBuffer = vertexBuffer;
        mIndexBuffer = indexBuffer;
    }

private:
    void ResizeWorldBufferIfNeeded(u32 objectCount);

private:
    SharedState *mSharedState;
    u32 mSharedStateIndex = 0;

    Vulkan::Buffer *mVertexBuffer = nullptr;
    Vulkan::Buffer *mIndexBuffer = nullptr;

    std::unique_ptr<Vulkan::Buffer> mWorldBuffer;
    std::unique_ptr<Vulkan::Buffer> mPerFrameBuffer;

    bool mIsDirty = true;
};

} // namespace BasicRendering
} // namespace Systems
