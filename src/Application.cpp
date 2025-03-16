#include "Application.h"

#include "Check.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/Renderer.h"

#include "Utils/Vertex.h"

#include "GLFW/glfw3.h"
#include "Gameplay/Game.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"

#include "vulkan/vulkan_core.h"

/* Redirects for callbacks */
void OnResizeCallback(GLFWwindow *window, int width, int height)
{
    Application::Get()->OnResize(width, height);
}

Application::Application() : mWidth(1280), mHeight(720)
{
    InitWindow();
    Vulkan::Renderer::Get(GetRendererCreateInfo());
    SetMouseInputMode(false);
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
    glfwSetWindowSizeCallback(mWindow, OnResizeCallback);

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

glm::vec2 Application::GetMouseRelativePosition()
{
    static glm::vec2 mLastMousePosition = GetMousePosition();
    auto mousePosition = GetMousePosition();
    auto mouseMovement = mousePosition - mLastMousePosition;
    if (mouseMovement != glm::vec2{0.0f, 0.0f})
    {
        mouseMovement /= GetWindowDimensions();
        mLastMousePosition = mousePosition;
    }
    return mouseMovement;
}

void Application::Frame()
{
    auto *game = Game::Get();
    game->Update();
    game->Render();

    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(mWindow, 1);
    }
}

void Application::OnResize(u32 width, u32 height)
{
    if (width < 10 || height < 10)
    {
        SHOWINFO(
            "The window is too small for a resize. Skipping resize callback");
        mMinimized = true;
        return;
    }
    mMinimized = false;

    mWidth = width;
    mHeight = height;

    Vulkan::Renderer::Get()->WaitIdle();
    Vulkan::Renderer::Get()->OnResize();
}

bool Application::IsKeyPressed(int keyCode)
{
    return glfwGetKey(mWindow, keyCode) == GLFW_PRESS;
}

bool Application::IsMousePressed(int keyCode)
{
    return glfwGetMouseButton(mWindow, keyCode) == GLFW_PRESS;
}

void Application::SetMouseInputMode(bool enable)
{
    if (enable)
    {
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(mWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

bool Application::IsMouseEnabled()
{
    return glfwGetInputMode(mWindow, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

glm::vec2 Application::GetMousePosition()
{
    double xpos = 0.0, ypos = 0.0;
    glfwGetCursorPos(mWindow, &xpos, &ypos);
    return {xpos, ypos};
}

glm::vec2 Application::GetWindowDimensions()
{
    return glm::vec2{mWidth, mHeight};
}

void Application::PostInit()
{
    Vulkan::CommandList cmdList(Vulkan::CommandListType::Graphics);
    cmdList.Init();
    cmdList.Begin();
    Game::Get(&cmdList);
    cmdList.End();
    cmdList.SubmitAndWait();
}

void Application::Run()
{
    PostInit();
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        Frame();
    }
    Destroy();
}

void Application::Destroy()
{
    auto renderer = Vulkan::Renderer::Get();
    renderer->WaitIdle();

    Game::Destroy();

    Vulkan::Renderer::Destroy();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}
