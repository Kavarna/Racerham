#pragma once

#include "Gameplay/Systems/BasicRendering.h"
#include "Gameplay/Systems/Physics.h"
#include "Gameplay/Systems/UpdateFrame.h"
#include "Singletone.h"
#include "entt/entt.hpp"

#include "Utils/Constants.h"
#include "Utils/Vertex.h"

#include "Gameplay/Camera.h"
#include "Gameplay/Components/Mesh.h"
#include "Gameplay/Entity.h"

#include "Renderer/Vulkan/Buffer.h"
#include "Renderer/Vulkan/CommandList.h"
#include "Renderer/Vulkan/Pipeline.h"
#include "Renderer/Vulkan/RootSignature.h"
#include "Renderer/Vulkan/SynchronizationObjects.h"
#include <string_view>

class Game : public Jnrlib::ISingletone<Game>
{
    MAKE_SINGLETONE_CAPABLE(Game);

private:
    Game(Vulkan::CommandList *initCommandList);
    ~Game();

public:
    void OnResize();

    void Update(float dt);
    void Render();

private:
    void InitPerFrameResources();
    void InitScene(Vulkan::CommandList *initCommandList);
    void InitResources();

    void DestroyFrameResources();

    Components::Mesh InitGeometry(std::string_view path);

private:
    void BakeRenderingBuffers(Vulkan::CommandList *initCommandList);

    Entity *AddTestEntity(std::string_view name);
    Entity *AddGround();

private:
    struct PerFrameResource
    {
        std::unique_ptr<Vulkan::CommandList> commandList;
        std::unique_ptr<Vulkan::CPUSynchronizationObject> isCommandListDone;

        std::unique_ptr<Systems::BasicRendering::RenderSystem>
            basicRenderSystem;
    };
    std::array<PerFrameResource, Constants::MAX_IN_FLIGHT_FRAMES>
        mPerFrameResources;
    u32 mCurrentFrame = 0;

    Systems::BasicRendering::SharedState mState;

    std::unique_ptr<Vulkan::Buffer> mGlobalVertexBuffer;
    std::unique_ptr<Vulkan::Buffer> mGlobalIndexBuffer;
    std::vector<VertexPositionNormal> mStagedVertexBuffer;
    std::vector<u32> mStagedIndexBuffer;

    Systems::UpdateFrame mUpdateFrameSystem;
    Systems::Physics mPhysicsSystem;

    Camera mCamera;

    /* TODO: Ideally merge these two into a single class */
    entt::registry mRegistry;
    std::vector<std::unique_ptr<Entity>> mEntities;
};
