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
    BatchRenderer()
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
    void Render(Vulkan::CommandList *cmdList, Camera const &camera);
    void AddVertex(VertexPositionColor const &vertex);

    void OnResize();

private:
    void InitVulkanState();
    void Resize(u32 newCount);

private:
    u32 mVertexCount = 0;
    std::unique_ptr<Vulkan::Buffer> mBuffer;
    std::unique_ptr<Vulkan::Pipeline> mPipeline;
    std::unique_ptr<Vulkan::RootSignature> mRootSignature;
};
