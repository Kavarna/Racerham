#pragma once

#include "Jnrlib.h"
#include "Utils/Vertex.h"

#include "Renderer/Vulkan/Buffer.h"
#include "Renderer/Vulkan/Pipeline.h"
#include "Renderer/Vulkan/RootSignature.h"

#include "Gameplay/Camera.h"

class BatchRenderer
{
public:
    BatchRenderer() : mPipeline("BatchRendererPipeline")
    {
        InitVulkanState();
        Resize(1024);
    };

    ~BatchRenderer() = default;

public:
    void Clear()
    {
        mVertexCount = 0;
    }
    void Render(Vulkan::CommandList &cmdList, Camera const &camera);
    void AddVertex(VertexPositionColor const &vertex);

    void OnResize();

private:
    void InitVulkanState();
    void Resize(u32 newCount);

private:
    u32 mVertexCount = 0;
    Vulkan::Buffer mVertexBuffer;
    Vulkan::Pipeline mPipeline;
    Vulkan::RootSignature mRootSignature;
};
