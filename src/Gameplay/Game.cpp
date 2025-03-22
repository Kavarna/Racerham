#include "Game.h"
#include "Application.h"

#include "Check.h"
#include "Exceptions.h"
#include "GLFW/glfw3.h"
#include "Gameplay/Components/Mesh.h"
#include "Gameplay/Components/RigidBody.h"
#include "Gameplay/Components/Update.h"
#include "Gameplay/PhysicsDebugDraw.h"
#include "Gameplay/Systems/BasicRendering.h"
#include "Gameplay/Systems/Physics.h"
#include "Renderer/Vulkan/Buffer.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/MemoryAllocator.h"
#include "Utils/Constants.h"
#include "Utils/Vertex.h"

#include "Components/Base.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "vulkan/vulkan_core.h"
#include <string_view>

Game::Game(Vulkan::CommandList *initCommandList)
{
    InitPerFrameResources();
    InitScene(initCommandList);
    InitResources();
#if DEBUG
    mPhysicsDebug = PhysicsDebugDraw(&mBatchRenderer);
    mPhysicsSystem.SetDebugInterface(&mPhysicsDebug);
#endif /* DEBUG */
}

Game::~Game()
{
    DestroyFrameResources();

    mEntities.clear();
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
    AddGround();
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

void Game::OnResize()
{
    mState.OnResize();
    mBatchRenderer.OnResize();
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
    else if (path == "cube")
    {
        static bool initialized = false;
        static Components::Mesh mesh = {};
        if (initialized)
        {
            return mesh;
        }

        u32 firstVertex = (u32)mStagedVertexBuffer.size();
        u32 firstIndex = (u32)mStagedIndexBuffer.size();

        mStagedVertexBuffer.reserve(mStagedVertexBuffer.size() + 8);
        mStagedIndexBuffer.reserve(mStagedIndexBuffer.size() + 36);

        // Define cube vertices (position + normal)
        mStagedVertexBuffer.emplace_back(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                                         -1.0f);
        mStagedVertexBuffer.emplace_back(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f);
        mStagedVertexBuffer.emplace_back(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f);
        mStagedVertexBuffer.emplace_back(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f);

        mStagedVertexBuffer.emplace_back(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
        mStagedVertexBuffer.emplace_back(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
        mStagedVertexBuffer.emplace_back(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
        mStagedVertexBuffer.emplace_back(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

        // Define cube indices (two triangles per face, six faces)
        u32 indices[] = {
            0, 1, 2, 0, 2, 3, // Back face
            4, 5, 6, 4, 6, 7, // Front face
            0, 1, 5, 0, 5, 4, // Bottom face
            3, 2, 6, 3, 6, 7, // Top face
            0, 3, 7, 0, 7, 4, // Left face
            1, 2, 6, 1, 6, 5  // Right face
        };

        mStagedIndexBuffer.insert(mStagedIndexBuffer.end(), std::begin(indices),
                                  std::end(indices));

        mesh.path = path;
        mesh.indices.firstIndex = firstIndex;
        mesh.indices.firstVertex = firstVertex;
        mesh.indices.indexCount = 36;
        mesh.indices.vertexCount = 8;

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

    glm::mat4x4 world = glm::identity<glm::mat4x4>();
    world = glm::translate(world, glm::vec3(0.0f, 5.0f, 0.0f));
    entity->AddComponent(Components::Base{.world = world,
                                          .name = std::string(name),
                                          .inverseWorld = glm::inverse(world),
                                          .entityPtr = entity.get()});
    entity->AddComponent(
        Components::Update{.dirtyFrames = Constants::MAX_IN_FLIGHT_FRAMES,
                           .bufferIndex = (u32)mEntities.size()});
    mRegistry.on_update<Components::Base>().connect<&Entity::UpdateBase>(
        entity.get());
    entity->UpdateBase();

    entity->AddComponent(InitGeometry("cube"));
    entity->AddComponent(mPhysicsSystem.CreateRigidBody(
        entity->GetComponent<Components::Base>(),
        entity->GetComponent<Components::Mesh>(), 1.0f));

    mEntities.push_back(std::move(entity));

    return mEntities.back().get();
}

Entity *Game::AddGround()
{
    std::unique_ptr<Entity> entity =
        std::make_unique<Entity>(mRegistry.create(), mRegistry);

    glm::mat4x4 world = glm::identity<glm::mat4x4>();
    world = glm::translate(world, glm::vec3(0.0f, -50.0f, 0.0f));
    world = glm::scale(world, glm::vec3(50.f, 50.f, 50.f));
    entity->AddComponent(Components::Base{.world = world,
                                          .name = std::string("Ground"),
                                          .inverseWorld = glm::inverse(world),
                                          .entityPtr = entity.get()});
    entity->AddComponent(
        Components::Update{.dirtyFrames = Constants::MAX_IN_FLIGHT_FRAMES,
                           .bufferIndex = (u32)mEntities.size()});
    mRegistry.on_update<Components::Base>().connect<&Entity::UpdateBase>(
        entity.get());
    entity->UpdateBase();

    entity->AddComponent(InitGeometry("cube"));
    entity->AddComponent(mPhysicsSystem.CreateRigidBody(
        entity->GetComponent<Components::Base>(),
        entity->GetComponent<Components::Mesh>(), 0.0f));

    mEntities.push_back(std::move(entity));

    return mEntities.back().get();
}

void Game::Update(float dt)
{
    auto &perFrameResources = mPerFrameResources[mCurrentFrame];
    auto *application = Application::Get();

    static float totalTime = 0.0f;
    totalTime += dt;
    if (totalTime >= 1.0f)
    {
        totalTime -= 1.0f;
    }

    constexpr float cameraSpeed = 5.0f;
    if (application->IsKeyPressed(GLFW_KEY_W) ||
        application->IsKeyPressed(GLFW_KEY_UP))
    {
        mCamera.MoveForward(dt * cameraSpeed);
    }
    if (application->IsKeyPressed(GLFW_KEY_S) ||
        application->IsKeyPressed(GLFW_KEY_DOWN))
    {
        mCamera.MoveBackward(dt * cameraSpeed);
    }
    if (application->IsKeyPressed(GLFW_KEY_A) ||
        application->IsKeyPressed(GLFW_KEY_LEFT))
    {
        mCamera.StrafeLeft(dt * cameraSpeed);
    }
    if (application->IsKeyPressed(GLFW_KEY_D) ||
        application->IsKeyPressed(GLFW_KEY_RIGHT))
    {
        mCamera.StrafeRight(dt * cameraSpeed);
    }

    if (application->IsKeyPressed(GLFW_KEY_SPACE))
    {
        auto &rigidBody = mEntities[0]->GetComponent<Components::RigidBody>();
        rigidBody.rigidBody->activate(true);
        rigidBody.rigidBody->applyCentralImpulse(btVector3(0.003f, 0.1f, 0.0f));
        btVector3 spin(0, 5, 5); // Spin around the Y-axis
        rigidBody.rigidBody->setAngularVelocity(spin);
    }

    auto mouseMovement = application->GetMouseRelativePosition();
    mCamera.Pitch(mouseMovement.y);
    mCamera.Yaw(mouseMovement.x);

    /* Update the camera */
    if (mCamera.Update())
    {
        perFrameResources.basicRenderSystem->UpdateCamera(mCamera);
    }

    mPhysicsSystem.Update(dt, mRegistry);
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
        mBatchRenderer.Render(cmdList.get(), mCamera);
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
