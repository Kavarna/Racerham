#pragma once

#include "GLFW/glfw3.h"
#include "Renderer/Vulkan/Renderer.h"
#include "src/Renderer/Vulkan/Buffer.h"
#include "src/Renderer/Vulkan/CommandList.h"
#include "src/Renderer/Vulkan/Pipeline.h"
#include "src/Renderer/Vulkan/SynchronizationObjects.h"
#include <Jnrlib/Singletone.h>

#include "LinearMath/btVector3.h"

class Application : public Jnrlib::ISingletone<Application>
{
    MAKE_SINGLETONE_CAPABLE(Application);
    static constexpr u32 MAX_IN_FLIGHT_FRAMES = 3;

private:
    Application();
    ~Application();

public:
    void Run();

private:
    void InitWindow();
    Vulkan::VulkanRendererInfo GetRendererCreateInfo();
    void InitPerFrameResources();
    void InitResources();

    void Destroy();
    void DestroyFrameResources();

private:
    void Frame();

private:
    struct PerFrameResource
    {
        std::unique_ptr<Vulkan::CommandList> commandList;
        std::unique_ptr<Vulkan::CPUSynchronizationObject> isCommandListDone;
    };

private:
    std::array<PerFrameResource, MAX_IN_FLIGHT_FRAMES> mPerFrameResources;
    u32 mCurrentFrame = 0;

    std::unique_ptr<Vulkan::Pipeline> mSimplePipeline;
    std::unique_ptr<Vulkan::Buffer> mVertexBuffer;

    GLFWwindow *mWindow;

    uint32_t mWidth;
    uint32_t mHeight;
};
