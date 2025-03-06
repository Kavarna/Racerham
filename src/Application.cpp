#include "Application.h"

#include "GLFW/glfw3.h"
#include "Jnrlib.h"
#include "src/Renderer/Vulkan/CommandList.h"
#include "src/Renderer/Vulkan/Renderer.h"
#include "src/Renderer/Vulkan/SynchronizationObjects.h"
#include <algorithm>
#include <memory>

Application::Application() : mWidth(1280), mHeight(720)
{
    InitWindow();
    Vulkan::Renderer::Get(GetRendererCreateInfo());
    InitPerFrameResources();
}

Application::~Application()
{
}

void Application::InitWindow()
{
    ThrowIfFailed(glfwInit() == GLFW_TRUE, "Unable to init glfw");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(mWidth, mHeight, "JNReditor", nullptr, nullptr);
    ThrowIfFailed(mWindow != nullptr, "Unable to create window");

    glfwMakeContextCurrent(mWindow);
    // glfwSetWindowSizeCallback(mWindow, OnResizeCallback);
    // glfwSetWindowMaximizeCallback(mWindow, OnMaximizeCallback);

    DSHOWINFO("Successfully created window");
}

Vulkan::VulkanRendererInfo Application::GetRendererCreateInfo()
{
    Vulkan::VulkanRendererInfo rendererInfo = {};
    {
        rendererInfo.window = mWindow;
        rendererInfo.deviceExtensions.emplace_back(
            VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        rendererInfo.deviceExtensions.emplace_back(
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    }

    {
        u32 count;
        const char **extensions = glfwGetRequiredInstanceExtensions(&count);

        for (u32 i = 0; i < count; ++i)
        {
            rendererInfo.instanceExtensions.emplace_back(extensions[i]);
        }
    }

#if DEBUG
    {
        DSHOWINFO("Enabling vulkan validation layers");
        rendererInfo.instanceExtensions.emplace_back(
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        rendererInfo.instanceLayers.emplace_back("VK_LAYER_KHRONOS_validation",
                                                 false);
    }
#endif

    return rendererInfo;
}

void Application::InitPerFrameResources()
{
    for (u32 i = 0; i < MAX_IN_FLIGHT_FRAMES; ++i)
    {
        mPerFrameResources[i].commandList =
            std::make_unique<Vulkan::CommandList>(
                Vulkan::CommandListType::Graphics);
        mPerFrameResources[i].commandList->Init();
        mPerFrameResources[i].isCommandListDone =
            std::make_unique<Vulkan::CPUSynchronizationObject>();
    }
}

void Application::Frame()
{
    auto &cmdList = mPerFrameResources[mCurrentFrame].commandList;
    auto &isCmdListDone = mPerFrameResources[mCurrentFrame].isCommandListDone;

    f32 backgroundColor[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    cmdList->BeginRenderingOnBackbuffer(backgroundColor);
    cmdList->EndRendering();

    cmdList->SubmitToScreen(isCmdListDone.get());

    mCurrentFrame = (mCurrentFrame + 1) % MAX_IN_FLIGHT_FRAMES;
}

void Application::Run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        Frame();
    }
    Destroy();
}

void Application::Destroy()
{
    Vulkan::Renderer::Destroy();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}
