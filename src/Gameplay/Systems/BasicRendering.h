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
    void Render(Vulkan::CommandList &cmdList, u32 currentFrameIndex, entt::registry const &registry, u32 objectCount);
    void UpdateCamera(Camera const &camera);

public:
    void SetRenderingBuffers(Vulkan::Buffer *vertexBuffer, Vulkan::Buffer *indexBuffer)
    {
        mVertexBuffer = vertexBuffer;
        mIndexBuffer = indexBuffer;
    }

    void SetDirectionalLight(Vulkan::CommandList &cmdList, glm::vec3 direction, glm::vec4 color, glm::vec4 ambient)
    {
        mPerSceneBuffer = Vulkan::Buffer(sizeof(PerSceneBuffer), 1,
                                         VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                             VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        Vulkan::Buffer stagingBuffer =
            Vulkan::Buffer(sizeof(PerSceneBuffer), 1, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        auto *data = static_cast<PerSceneBuffer *>(stagingBuffer.GetData());
        data->ambient = ambient;
        data->color = color;
        data->direction = glm::normalize(direction);

        cmdList.CopyBuffer(mPerSceneBuffer, stagingBuffer);
        cmdList.AddLocalBuffer(std::move(stagingBuffer));
    }

private:
    struct PerSceneBuffer
    {
        glm::vec3 color;
        float pad0;
        glm::vec3 ambient;
        float pad1;
        glm::vec3 direction;
    };

private:
    void StateInit();
    void ResizeWorldBufferIfNeeded(u32 objectCount);

private:
    Vulkan::Pipeline mPipeline;
    Vulkan::RootSignature mRootSignature;
    Vulkan::DescriptorSet mDescriptorSet;

    Vulkan::Buffer *mVertexBuffer = nullptr;
    Vulkan::Buffer *mIndexBuffer = nullptr;

    Vulkan::Buffer mWorldBuffer;
    Vulkan::Buffer mPerFrameBuffer;
    Vulkan::Buffer mPerSceneBuffer;

    bool mIsDirty = true;
};

} // namespace BasicRendering
} // namespace Systems
