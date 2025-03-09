#include "Application.h"

#include "Check.h"
#include "Gameplay/Camera.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/Pipeline.h"
#include "Renderer/Vulkan/Renderer.h"
#include "Renderer/Vulkan/RootSignature.h"
#include "Renderer/Vulkan/SynchronizationObjects.h"

#include "Utils/Vertex.h"

#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/fwd.hpp"
#include "vulkan/vulkan_core.h"

#include <memory>

/* Redirects for callbacks */
void OnResizeCallback(GLFWwindow *window, int width, int height)
{
    Application::Get()->OnResize(width, height);
}

Application::Application() : mWidth(1280), mHeight(720)
{
    InitWindow();
    Vulkan::Renderer::Get(GetRendererCreateInfo());
    InitPerFrameResources();
    InitResources();
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

void Application::InitPerFrameResources()
{
    for (u32 i = 0; i < Constants::MAX_IN_FLIGHT_FRAMES; ++i)
    {
        mPerFrameResources[i].commandList =
            std::make_unique<Vulkan::CommandList>(
                Vulkan::CommandListType::Graphics);
        mPerFrameResources[i].commandList->Init();
        mPerFrameResources[i].isCommandListDone =
            std::make_unique<Vulkan::CPUSynchronizationObject>(true);
        mPerFrameResources[i].cameraBuffer = std::make_unique<Vulkan::Buffer>(
            sizeof(glm::mat4x4), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    }
}

void Application::InitResources()
{
    /* Create simple camera */
    mCamera = Camera(glm::vec3(0.0f, 0.0f, -5.0f), (f32)mWidth, (f32)mHeight,
                     glm::pi<float>() / 4);

    /* Create vertex buffer */
    float vertices[] = {0.0f, 1.0f, 0.0f,  -1.0f, -1.0f,
                        0.0f, 1.0f, -1.0f, 0.0f};

    mVertexBuffer = std::make_unique<Vulkan::Buffer>(
        sizeof(float) * 3, 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    mVertexBuffer->Copy(vertices);

    /* Create a simple root signature*/
    mDescriptorSet = std::make_unique<Vulkan::DescriptorSet>();
    {
        mDescriptorSet->AddStorageBuffer(0, 1);
        mDescriptorSet->AddInputBuffer(1, 1);
        mDescriptorSet->Bake();
    }
    mRootSignature = std::make_unique<Vulkan::RootSignature>();
    {
        mRootSignature->AddPushRange<u32>(0, 1);
        mRootSignature->AddDescriptorSet(mDescriptorSet.get());
    }
    mRootSignature->Bake();

    mWorldBuffer = std::make_unique<Vulkan::Buffer>(
        sizeof(glm::mat4x4), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    auto identityMatrix = glm::identity<glm::mat4x4>();
    mWorldBuffer->Copy(&identityMatrix);

    mDescriptorSet->BindStorageBuffer(mWorldBuffer.get(), 0);
    mDescriptorSet->BindInputBuffer(mPerFrameResources[0].cameraBuffer.get(),
                                    1);

    /* Create simple pipeline */
    VkViewport viewport = {};
    {
        viewport.width = (float)mWidth;
        viewport.height = (float)mHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.x = 0;
        viewport.y = 0;
    }
    VkRect2D scissor = {};
    {
        scissor.offset = {0, 0};
        scissor.extent = {mWidth, mHeight};
    }

    mSimplePipeline = std::make_unique<Vulkan::Pipeline>("SimplePipeline");
    {
        mSimplePipeline->SetRootSignature(mRootSignature.get());
        mSimplePipeline->AddShader("Shaders/basic.vert.spv");
        mSimplePipeline->AddShader("Shaders/basic.frag.spv");
    }
    {
        auto &viewportState = mSimplePipeline->GetViewportStateCreateInfo();
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
    }
    {
        auto &rasterizationState =
            mSimplePipeline->GetRasterizationStateCreateInfo();
        rasterizationState.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;
    }
    VkPipelineColorBlendAttachmentState attachmentInfo{};
    {
        auto &blendState = mSimplePipeline->GetColorBlendStateCreateInfo();
        attachmentInfo.blendEnable = VK_FALSE;
        attachmentInfo.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        blendState.attachmentCount = 1;
        blendState.pAttachments = &attachmentInfo;
    }
    auto vertexPositionAttributeDescription =
        VertexPosition::GetInputAttributeDescription();
    auto vertexPositionBindingDescription =
        VertexPosition::GetInputBindingDescription();
    {
        auto &vertexInput = mSimplePipeline->GetVertexInputStateCreateInfo();
        vertexInput.vertexAttributeDescriptionCount =
            (u32)vertexPositionAttributeDescription.size();
        vertexInput.pVertexAttributeDescriptions =
            vertexPositionAttributeDescription.data();
        vertexInput.vertexBindingDescriptionCount =
            (u32)vertexPositionBindingDescription.size();
        vertexInput.pVertexBindingDescriptions =
            vertexPositionBindingDescription.data();
    }
    mSimplePipeline->AddBackbufferColorOutput();
    mSimplePipeline->Bake();
}

void Application::Frame()
{
    auto &cmdList = mPerFrameResources[mCurrentFrame].commandList;
    auto &isCmdListDone = mPerFrameResources[mCurrentFrame].isCommandListDone;
    auto &cameraBuffer = mPerFrameResources[mCurrentFrame].cameraBuffer;
    /* Update the camera */
    mCamera.Update();

    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(mWindow, 1);
    }

    if (IsKeyPressed(GLFW_KEY_W) || IsKeyPressed(GLFW_KEY_UP))
    {
        mCamera.MoveForward(0.0016f);
    }
    if (IsKeyPressed(GLFW_KEY_S) || IsKeyPressed(GLFW_KEY_DOWN))
    {
        mCamera.MoveBackward(0.0016f);
    }
    if (IsKeyPressed(GLFW_KEY_A) || IsKeyPressed(GLFW_KEY_LEFT))
    {
        mCamera.StrafeLeft(0.0016f);
    }
    if (IsKeyPressed(GLFW_KEY_D) || IsKeyPressed(GLFW_KEY_RIGHT))
    {
        mCamera.StrafeRight(0.0016f);
    }

    static glm::vec2 mLastMousePosition = GetMousePosition();
    auto mousePosition = GetMousePosition();
    auto mouseMovement = mousePosition - mLastMousePosition;
    if (mouseMovement != glm::vec2{0.0f, 0.0f})
    {
        mouseMovement /= GetWindowDimensions();
        mCamera.Pitch(mouseMovement.y);
        mCamera.Yaw(mouseMovement.x);
        mLastMousePosition = mousePosition;
    }

    auto viewProjectionMatrix = mCamera.GetProjection() * mCamera.GetView();
    // auto viewProjectionMatrix = glm::identity<glm::mat4x4>();
    cameraBuffer->Copy(&viewProjectionMatrix);

    isCmdListDone->Wait();
    isCmdListDone->Reset();

    cmdList->Begin();

    {
        f32 backgroundColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cmdList->BeginRenderingOnBackbuffer(backgroundColor);

        {
            cmdList->BindVertexBuffer(mVertexBuffer.get(), 0);
            cmdList->BindPipeline(mSimplePipeline.get());
            cmdList->BindDescriptorSet(mDescriptorSet.get(), 0,
                                       mRootSignature.get());
            u32 index = 0;
            cmdList->BindPushRange<u32>(mRootSignature.get(), 0, 1, &index);
            cmdList->Draw(3, 0);
        }

        cmdList->EndRendering();
    }

    cmdList->End();

    cmdList->SubmitToScreen(isCmdListDone.get());

    mCurrentFrame = (mCurrentFrame + 1) % Constants::MAX_IN_FLIGHT_FRAMES;
}

void Application::OnResize(uint32_t width, uint32_t height)
{
    if (width < 5 || height < 5)
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

void Application::Run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        Frame();
    }
    Destroy();
}

void Application::DestroyFrameResources()
{
    for (u32 i = 0; i < Constants::MAX_IN_FLIGHT_FRAMES; ++i)
    {
        mPerFrameResources[i].commandList.reset();
        mPerFrameResources[i].isCommandListDone.reset();
        mPerFrameResources[i].cameraBuffer.reset();
    }
}

void Application::Destroy()
{
    auto renderer = Vulkan::Renderer::Get();
    renderer->WaitIdle();

    DestroyFrameResources();
    mWorldBuffer.reset();
    mDescriptorSet.reset();
    mRootSignature.reset();
    mSimplePipeline.reset();
    mVertexBuffer.reset();

    Vulkan::Renderer::Destroy();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}
