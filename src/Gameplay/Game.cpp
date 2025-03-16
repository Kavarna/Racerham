#include "Game.h"
#include "Application.h"

#include "Check.h"
#include "Exceptions.h"
#include "Gameplay/Components/Mesh.h"
#include "Gameplay/Components/Update.h"
#include "Gameplay/Systems/BasicRendering.h"
#include "Renderer/Vulkan/Buffer.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/MemoryAllocator.h"
#include "Utils/Constants.h"
#include "Utils/Vertex.h"

#include "Components/Base.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "vulkan/vulkan_core.h"
#include <string_view>

Game::Game(Vulkan::CommandList *initCommandList)
{
    InitPerFrameResources();
    InitScene(initCommandList);
    InitResources();
}

Game::~Game()
{
    DestroyFrameResources();

    mGlobalVertexBuffer.reset();
}

void Game::InitPerFrameResources()
{
    for (u32 i = 0; i < Constants::MAX_IN_FLIGHT_FRAMES; ++i)
    {
        mPerFrameResources[i].commandList =
            std::make_unique<Vulkan::CommandList>(
                Vulkan::CommandListType::Graphics);
        mPerFrameResources[i].commandList->Init();
        mPerFrameResources[i].isCommandListDone =
            std::make_unique<Vulkan::CPUSynchronizationObject>(true);
        mPerFrameResources[i].basicRenderSystem =
            std::make_unique<Systems::BasicRendering::RenderSystem>(&mState, i);
    }
}

void Game::InitScene(Vulkan::CommandList *initCommandList)
{
    AddTestEntity("TestEntity1");
    BakeRenderingBuffers(initCommandList);
}

void Game::BakeRenderingBuffers(Vulkan::CommandList *initCommandList)
{
    /* Create staged buffers and copy data into them */
    std::unique_ptr<Vulkan::Buffer> stagingVertexBuffer =
        std::make_unique<Vulkan::Buffer>(
            sizeof(VertexPositionNormal), mStagedVertexBuffer.size(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    stagingVertexBuffer->Copy(mStagedVertexBuffer.data());

    std::unique_ptr<Vulkan::Buffer> stagingIndexBuffer =
        std::make_unique<Vulkan::Buffer>(
            sizeof(u32), mStagedIndexBuffer.size(),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    stagingIndexBuffer->Copy(mStagedIndexBuffer.data());

    /* Create vertex & index buffer */
    mGlobalVertexBuffer = std::make_unique<Vulkan::Buffer>(
        sizeof(VertexPositionNormal), mStagedVertexBuffer.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    mGlobalIndexBuffer = std::make_unique<Vulkan::Buffer>(
        sizeof(u32), mStagedIndexBuffer.size(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    /* Perfom copy from staged buffer to actual buffers */
    initCommandList->CopyBuffer(mGlobalVertexBuffer.get(),
                                stagingVertexBuffer.get());
    initCommandList->CopyBuffer(mGlobalIndexBuffer.get(),
                                stagingIndexBuffer.get());

    /* Save temporary buffers in order to safely delete them later */
    initCommandList->AddLocalBuffer(std::move(stagingVertexBuffer));
    initCommandList->AddLocalBuffer(std::move(stagingIndexBuffer));

    for (u32 i = 0; i < Constants::MAX_IN_FLIGHT_FRAMES; ++i)
    {
        mPerFrameResources[i].basicRenderSystem->SetRenderingBuffers(
            mGlobalVertexBuffer.get(), mGlobalIndexBuffer.get());
    }
}

void Game::InitResources()
{
    glm::vec2 windowDimensions = Application::Get()->GetWindowDimensions();
    /* Create simple camera */
    mCamera = Camera(glm::vec3(0.0f, 0.0f, -5.0f), (f32)windowDimensions.x,
                     (f32)windowDimensions.y, glm::pi<float>() / 4);
}

Components::Mesh Game::InitGeometry(std::string_view path)
{
    if (path == "quad")
    {
        static bool initialized = false;
        static Components::Mesh mesh = {};
        if (initialized)
        {
            return mesh;
        }

        u32 firstVertex = (u32)mStagedVertexBuffer.size();
        u32 firstIndex = (u32)mStagedIndexBuffer.size();

        mStagedVertexBuffer.reserve(mStagedVertexBuffer.size() + 4);
        mStagedIndexBuffer.reserve(mStagedIndexBuffer.size() + 6);

        mStagedVertexBuffer.emplace_back(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
        mStagedVertexBuffer.emplace_back(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
        mStagedVertexBuffer.emplace_back(1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
        mStagedVertexBuffer.emplace_back(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
        mStagedIndexBuffer.emplace_back(0);
        mStagedIndexBuffer.emplace_back(1);
        mStagedIndexBuffer.emplace_back(2);
        mStagedIndexBuffer.emplace_back(0);
        mStagedIndexBuffer.emplace_back(2);
        mStagedIndexBuffer.emplace_back(3);

        mesh.path = path;
        mesh.indices.firstIndex = firstIndex;
        mesh.indices.firstVertex = firstVertex;
        mesh.indices.indexCount = 6;
        mesh.indices.vertexCount = 3;

        return mesh;
    }

    /* Very javaeque of me */
    throw Jnrlib::Exceptions::JNRException(
        Format("Could not find mesh for path ", path));
    return {};
}

Entity *Game::AddTestEntity(std::string_view name)
{
    std::unique_ptr<Entity> entity =
        std::make_unique<Entity>(mRegistry.create(), mRegistry);

    entity->AddComponent(
        Components::Base{.world = glm::identity<glm::mat4x4>(),
                         .name = std::string(name),
                         .inverseWorld = glm::identity<glm::mat4x4>(),
                         .entityPtr = entity.get()});
    entity->AddComponent(
        Components::Update{.dirtyFrames = Constants::MAX_IN_FLIGHT_FRAMES,
                           .bufferIndex = (u32)mEntities.size()});
    mRegistry.on_update<Components::Base>().connect<&Entity::UpdateBase>(
        entity.get());
    entity->UpdateBase();

    entity->AddComponent(InitGeometry("quad"));

    mEntities.push_back(std::move(entity));

    return mEntities.back().get();
}

void Game::Update()
{
    auto &perFrameResources = mPerFrameResources[mCurrentFrame];
    auto *application = Application::Get();

    if (application->IsKeyPressed(GLFW_KEY_W) ||
        application->IsKeyPressed(GLFW_KEY_UP))
    {
        mCamera.MoveForward(0.0016f);
    }
    if (application->IsKeyPressed(GLFW_KEY_S) ||
        application->IsKeyPressed(GLFW_KEY_DOWN))
    {
        mCamera.MoveBackward(0.0016f);
    }
    if (application->IsKeyPressed(GLFW_KEY_A) ||
        application->IsKeyPressed(GLFW_KEY_LEFT))
    {
        mCamera.StrafeLeft(0.0016f);
    }
    if (application->IsKeyPressed(GLFW_KEY_D) ||
        application->IsKeyPressed(GLFW_KEY_RIGHT))
    {
        mCamera.StrafeRight(0.0016f);
    }

    auto mouseMovement = application->GetMouseRelativePosition();
    mCamera.Pitch(mouseMovement.y);
    mCamera.Yaw(mouseMovement.x);

    /* Update the camera */
    if (mCamera.Update())
    {
        perFrameResources.basicRenderSystem->UpdateCamera(mCamera);
    }

    if (application->IsKeyPressed(GLFW_KEY_P))
    {
        mRegistry.patch<Components::Base>(
            (entt::entity)mEntities[0]->GetEntityId(),
            [&](Components::Base &base) {
                base.world = glm::rotate(base.world, 0.00016f,
                                         glm::vec3(0.0f, 0.0f, 1.0f));
            });
    }
}

void Game::Render()
{
    auto &cmdList = mPerFrameResources[mCurrentFrame].commandList;
    auto &isCmdListDone = mPerFrameResources[mCurrentFrame].isCommandListDone;
    auto &basicRenderSystem =
        mPerFrameResources[mCurrentFrame].basicRenderSystem;

    isCmdListDone->Wait();
    isCmdListDone->Reset();

    cmdList->Begin();
    {
        f32 backgroundColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cmdList->BeginRenderingOnBackbuffer(backgroundColor);
        basicRenderSystem->Render(cmdList.get(), mRegistry, mEntities.size());
        cmdList->EndRendering();
    }
    cmdList->End();

    cmdList->SubmitToScreen(isCmdListDone.get());

    /* Finish the frame and update the dirty flag */
    mCurrentFrame = (mCurrentFrame + 1) % Constants::MAX_IN_FLIGHT_FRAMES;
    mUpdateFrameSystem.Update(mRegistry);
}

void Game::DestroyFrameResources()
{
    for (u32 i = 0; i < Constants::MAX_IN_FLIGHT_FRAMES; ++i)
    {
        mPerFrameResources[i].commandList.reset();
        mPerFrameResources[i].isCommandListDone.reset();
    }
}
