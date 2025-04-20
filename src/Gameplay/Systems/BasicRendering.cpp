#include "BasicRendering.h"

#include "Application.h"

#include "Check.h"
#include "Gameplay/Components/Base.h"
#include "Gameplay/Components/Mesh.h"
#include "Gameplay/Components/Update.h"
#include "Renderer/ShaderStructs.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Utils/Constants.h"
#include "Utils/Vertex.h"

namespace Systems
{
namespace BasicRendering
{

RenderSystem::RenderSystem() : mPipeline("SimplePipeline")
{
    mPerFrameBuffer = Vulkan::Buffer(
        sizeof(glm::mat4x4), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    StateInit();
}

void RenderSystem::StateInit()
{
    /* Create a simple root signature*/
    {
        mDescriptorSet.AddStorageBuffer(0, 1);
        mDescriptorSet.AddInputBuffer(1, 1);
        mDescriptorSet.Bake(Constants::MAX_IN_FLIGHT_FRAMES);
    }
    {
        mRootSignature.AddPushRange<u32>(0, 1);
        mRootSignature.AddDescriptorSet(&mDescriptorSet);
    }
    mRootSignature.Bake();
    OnResize();
}

void RenderSystem::OnResize()
{
    /* Create simple mPipeline */
    glm::vec2 windowDimensions = Application::Get()->GetWindowDimensions();
    VkViewport viewport = {};
    {
        viewport.width = windowDimensions.x;
        viewport.height = windowDimensions.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.x = 0;
        viewport.y = 0;
    }
    VkRect2D scissor = {};
    {
        scissor.offset = {0, 0};
        scissor.extent = {(u32)windowDimensions.x, (u32)windowDimensions.y};
    }

    mPipeline.Clear();
    {
        mPipeline.SetRootSignature(&mRootSignature);
        mPipeline.AddShader("Shaders/basic.vert.spv");
        mPipeline.AddShader("Shaders/basic.frag.spv");
    }
    {
        auto &viewportState = mPipeline.GetViewportStateCreateInfo();
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
    }
    {
        auto &rasterizationState = mPipeline.GetRasterizationStateCreateInfo();
        rasterizationState.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;
    }
    VkPipelineColorBlendAttachmentState attachmentInfo{};
    {
        auto &blendState = mPipeline.GetColorBlendStateCreateInfo();
        attachmentInfo.blendEnable = VK_FALSE;
        attachmentInfo.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        blendState.attachmentCount = 1;
        blendState.pAttachments = &attachmentInfo;
    }
    auto vertexPositionAttributeDescription =
        VertexPositionNormal::GetInputAttributeDescription();
    auto vertexPositionBindingDescription =
        VertexPositionNormal::GetInputBindingDescription();
    {
        auto &vertexInput = mPipeline.GetVertexInputStateCreateInfo();
        vertexInput.vertexAttributeDescriptionCount =
            (u32)vertexPositionAttributeDescription.size();
        vertexInput.pVertexAttributeDescriptions =
            vertexPositionAttributeDescription.data();
        vertexInput.vertexBindingDescriptionCount =
            (u32)vertexPositionBindingDescription.size();
        vertexInput.pVertexBindingDescriptions =
            vertexPositionBindingDescription.data();
    }
    {
        auto &depthState = mPipeline.GetDepthStencilStateCreateInfo();
        depthState.depthTestEnable = VK_TRUE;
        depthState.depthWriteEnable = VK_TRUE;
        depthState.depthBoundsTestEnable = VK_TRUE;
        depthState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthState.depthBoundsTestEnable = VK_TRUE;
        depthState.minDepthBounds = 0.0f;
        depthState.maxDepthBounds = 1.0f;
    }
    /* Render directly to the backbuffer and depth stencil but they must have
     * been bound before actully rendering */
    mPipeline.AddBackbufferColorOutput();
    mPipeline.SetBackbufferDepthStencilOutput();
    mPipeline.Bake();
}

void RenderSystem::UpdateCamera(Camera const &camera)
{
    auto viewProjectionMatrix = camera.GetProjection() * camera.GetView();
    mPerFrameBuffer.Copy(&viewProjectionMatrix);
    mIsDirty = true;
}
void RenderSystem::ResizeWorldBufferIfNeeded(u32 objectCount)
{
    if (objectCount > mWorldBuffer.GetCount()) [[unlikely]]
    {
        mWorldBuffer = Vulkan::Buffer(
            sizeof(BasicPerObjectInfo), objectCount,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        mIsDirty = true;
    }
}

void RenderSystem::Render(Vulkan::CommandList &cmdList, u32 currentFrameIndex,
                          entt::registry const &registry, u32 objectCount)
{
    ResizeWorldBufferIfNeeded(objectCount);

    auto updatables =
        registry.view<const Components::Base, const Components::Update>();
    for (auto const &[entity, base, update] : updatables.each())
    {
        if (update.dirtyFrames)
        {
            auto *info = (BasicPerObjectInfo *)mWorldBuffer.GetElement(
                update.bufferIndex);
            info->world = base.world;
            mIsDirty = true;
        }
    }

    CHECK_FATAL(mVertexBuffer, "A vertex buffer was not specified");
    CHECK_FATAL(mIndexBuffer, "A index buffer was not specified");

    if (mIsDirty) [[unlikely]]
    {
        /* TODO: To research if recording everything in a secondary command
         * buffer and just executing that instead of re-recording makes sense */

        mDescriptorSet.SetActiveInstance(currentFrameIndex);
        mDescriptorSet.BindStorageBuffer(mWorldBuffer, 0);
        mDescriptorSet.BindInputBuffer(mPerFrameBuffer, 1);
    }

    cmdList.BindVertexBuffer(*mVertexBuffer, 0);
    cmdList.BindIndexBuffer(*mIndexBuffer);
    cmdList.BindPipeline(mPipeline);
    cmdList.BindDescriptorSet(mDescriptorSet, currentFrameIndex,
                              mRootSignature);

    auto meshes =
        registry.view<const Components::Update, const Components::Mesh>();
    for (auto const &[entity, update, mesh] : meshes.each())
    {
        u32 index = update.bufferIndex;
        cmdList.BindPushRange<u32>(mRootSignature, 0, 1, &index);
        cmdList.DrawIndexedInstanced(mesh.indices.indexCount,
                                     mesh.indices.firstIndex,
                                     mesh.indices.firstVertex);
    }
}

} // namespace BasicRendering
} // namespace Systems
