#include "Application.h"

#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/Pipeline.h"
#include "Renderer/Vulkan/Renderer.h"
#include "Renderer/Vulkan/SynchronizationObjects.h"

#include "Renderer/Vertex.h"

#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

#include <cmath>
#include <memory>

Application::Application() : mWidth(1280), mHeight(720)
{
    InitWindow();
    Vulkan::Renderer::Get(GetRendererCreateInfo());
    InitPerFrameResources();
    InitResources();
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
            std::make_unique<Vulkan::CPUSynchronizationObject>(true);
    }
}

void Application::InitResources()
{
    /* Create vertex buffer */
    float vertices[] = {0.0f, 1.0f, 0.0f,  -1.0f, -1.0f,
                        0.0f, 1.0f, -1.0f, 0.0f};

    mVertexBuffer = std::make_unique<Vulkan::Buffer>(
        sizeof(float) * 3, 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    mVertexBuffer->Copy(vertices);

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
            (uint32_t)vertexPositionAttributeDescription.size();
        vertexInput.pVertexAttributeDescriptions =
            vertexPositionAttributeDescription.data();
        vertexInput.vertexBindingDescriptionCount =
            (uint32_t)vertexPositionBindingDescription.size();
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

    isCmdListDone->Wait();
    isCmdListDone->Reset();

    cmdList->Begin();

    {
        f32 backgroundColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cmdList->BeginRenderingOnBackbuffer(backgroundColor);

        {
            cmdList->BindVertexBuffer(mVertexBuffer.get(), 0);
            cmdList->BindPipeline(mSimplePipeline.get());
            cmdList->Draw(3, 0);
        }

        cmdList->EndRendering();
    }

    cmdList->End();

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

void Application::DestroyFrameResources()
{
    for (u32 i = 0; i < MAX_IN_FLIGHT_FRAMES; ++i)
    {
        mPerFrameResources[i].commandList.reset();
        mPerFrameResources[i].isCommandListDone.reset();
    }
}

void Application::Destroy()
{
    auto renderer = Vulkan::Renderer::Get();
    renderer->WaitIdle();

    DestroyFrameResources();
    mSimplePipeline.reset();
    mVertexBuffer.reset();

    Vulkan::Renderer::Destroy();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}
