#pragma once

#include "GLFW/glfw3.h"
#include "Gameplay/Camera.h"
#include "Renderer/Vulkan/Buffer.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/Pipeline.h"
#include "Renderer/Vulkan/Renderer.h"
#include "Renderer/Vulkan/RootSignature.h"
#include "Renderer/Vulkan/SynchronizationObjects.h"
#include <Jnrlib/Singletone.h>

#include <glm/glm.hpp>

#include "Utils/Constants.h"

class Application : public Jnrlib::ISingletone<Application>
{
    MAKE_SINGLETONE_CAPABLE(Application);

private:
    Application();
    ~Application();

public:
    void Run();

public:
    void OnResize(uint32_t width, uint32_t height);
    bool IsKeyPressed(int keyCode);
    bool IsMousePressed(int keyCode);

    void SetMouseInputMode(bool enable);
    bool IsMouseEnabled();

    glm::vec2 GetMousePosition();
    glm::vec2 GetWindowDimensions();

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
        std::unique_ptr<Vulkan::Buffer> cameraBuffer;
    };

private:
    std::array<PerFrameResource, Constants::MAX_IN_FLIGHT_FRAMES>
        mPerFrameResources;
    u32 mCurrentFrame = 0;

    std::unique_ptr<Vulkan::Pipeline> mSimplePipeline;
    std::unique_ptr<Vulkan::Buffer> mVertexBuffer;
    std::unique_ptr<Vulkan::RootSignature> mRootSignature;
    std::unique_ptr<Vulkan::DescriptorSet> mDescriptorSet;
    std::unique_ptr<Vulkan::Buffer> mWorldBuffer;

    Camera mCamera;

    GLFWwindow *mWindow;

    bool mMinimized = false;

    u32 mWidth;
    u32 mHeight;
};
