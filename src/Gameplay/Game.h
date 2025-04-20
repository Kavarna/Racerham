#pragma once

#include "Gameplay/PhysicsDebugDraw.h"
#include "Gameplay/Systems/BasicRendering.h"
#include "Gameplay/Systems/Physics.h"
#include "Gameplay/Systems/UpdateFrame.h"
#include "MemoryArena.h"
#include "Singletone.h"
#include "entt/entt.hpp"

#include "Utils/Constants.h"
#include "Utils/Vertex.h"

#include "Gameplay/Camera.h"
#include "Gameplay/Components/Mesh.h"
#include "Gameplay/Entity.h"

#include "Renderer/BatchRenderer.h"
#include "Renderer/Vulkan/Buffer.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/SynchronizationObjects.h"
#include <string_view>

struct GameState
{
    bool isDeveloper =
#if DEBUG
        true;
#else
        false;
#endif /* DEBUG */
};

class Game : public Jnrlib::ISingletone<Game>
{
    MAKE_SINGLETONE_CAPABLE(Game);

private:
    Game(Vulkan::CommandList &initCommandList);
    ~Game();

public:
    void OnResize();

    void Update(float dt);
    void Render();

    GameState const &GetGameState() const
    {
        return mState;
    }

private:
    void InitScene(Vulkan::CommandList &initCommandList);
    void InitSizeDependentResources();

    Components::Mesh InitGeometry(std::string_view path);

private:
    void BakeRenderingBuffers(Vulkan::CommandList &initCommandList);

    Entity *AddTestEntity(std::string_view name);
    Entity *AddGround();

private:
    struct PerFrameResource
    {
        Vulkan::CommandList commandList;
        Vulkan::CPUSynchronizationObject isCommandListDone;

        PerFrameResource()
            : commandList(Vulkan::CommandListType::Graphics),
              isCommandListDone(true)
        {
            commandList.Init();
        }
    };
    std::array<PerFrameResource, Constants::MAX_IN_FLIGHT_FRAMES>
        mPerFrameResources;
    u32 mCurrentFrame = 0;

    GameState mState;

    Vulkan::Buffer mGlobalVertexBuffer;
    Vulkan::Buffer mGlobalIndexBuffer;
    std::vector<VertexPositionNormal> mStagedVertexBuffer;
    std::vector<u32> mStagedIndexBuffer;

    Systems::UpdateFrame mUpdateFrameSystem;
    Systems::Physics mPhysicsSystem;
    Systems::BasicRendering::RenderSystem mBasicRenderSystem;

    Vulkan::Image mDepthImage;

    Camera mCamera;

    BatchRenderer mBatchRenderer;
    PhysicsDebugDraw mPhysicsDebug;

    /* TODO: Ideally merge these two into a single class */
    entt::registry mRegistry;
    std::vector<Entity *> mEntities;
    MemoryArena<1024, Entity> mEntityArena;
};
