#include "BatchRenderer.h"
#include "Application.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/MemoryAllocator.h"
#include "Utils/Vertex.h"
#include "vulkan/vulkan_core.h"
#include <cstring>
#include <memory>

void BatchRenderer::InitVulkanState()
{
    /* Create a simple root signature*/
    {
        mRootSignature.AddPushRange<glm::mat4x4>(0);
    }
    mRootSignature.Bake();
    OnResize();
}

void BatchRenderer::OnResize()
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
        mPipeline.AddShader("Shaders/color.vert.spv");
        mPipeline.AddShader("Shaders/color.frag.spv");
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
        rasterizationState.lineWidth = 3.0f;
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
        VertexPositionColor::GetInputAttributeDescription();
    auto vertexPositionBindingDescription =
        VertexPositionColor::GetInputBindingDescription();
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
    mPipeline.GetInputAssemblyStateCreateInfo().topology =
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    mPipeline.AddBackbufferColorOutput();
    mPipeline.SetBackbufferDepthStencilOutput();
    mPipeline.Bake();
}

void BatchRenderer::Resize(u32 newCount)
{
    auto newBuffer =
        Vulkan::Buffer(sizeof(VertexPositionColor), newCount,
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    if (mVertexBuffer.GetCount())
    {
        for (u32 i = 0; i < mVertexBuffer.GetCount(); ++i)
        {
            auto dst = newBuffer.GetElement(i);
            auto src = mVertexBuffer.GetElement(i);
            memcpy(dst, src, mVertexBuffer.GetElementSize());
        }
    }
    mVertexBuffer = std::move(newBuffer);
}

void BatchRenderer::Render(Vulkan::CommandList &cmdList, Camera const &camera)
{
    auto viewProj = camera.GetProjection() * camera.GetView();

    cmdList.BindPipeline(mPipeline);
    cmdList.BindVertexBuffer(mVertexBuffer, 0);
    cmdList.BindPushRange<glm::mat4x4>(mRootSignature, 0, 1, &viewProj);
    cmdList.Draw(mVertexCount, 0);

    Clear();
}

void BatchRenderer::AddVertex(VertexPositionColor const &vertex)
{
    if (mVertexCount > mVertexBuffer.GetCount())
    {
        Resize(mVertexBuffer.GetCount() * 2);
        AddVertex(vertex);
        return;
    }

    auto *element =
        (VertexPositionColor *)mVertexBuffer.GetElement(mVertexCount);
    *element = vertex;
    mVertexCount++;
}
