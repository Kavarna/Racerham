#pragma once

#include "GLFW/glfw3.h"
#include "Renderer/Vulkan/Renderer.h"
#include <Jnrlib/Singletone.h>

#include <glm/glm.hpp>

class Application : public Jnrlib::ISingletone<Application>
{
    MAKE_SINGLETONE_CAPABLE(Application);

private:
    Application();
    ~Application();

public:
    void Run();

public:
    void OnResize(u32 width, u32 height);
    bool IsKeyPressed(int keyCode);
    bool IsMousePressed(int keyCode);

    glm::vec2 GetMouseRelativePosition();

    void SetMouseInputMode(bool enable);
    bool IsMouseEnabled();

    glm::vec2 GetMousePosition();
    glm::vec2 GetWindowDimensions();

private:
    void InitWindow();
    Vulkan::VulkanRendererInfo GetRendererCreateInfo();

    void PostInit();

    void Destroy();

private:
    void Frame();

    GLFWwindow *mWindow;

    bool mMinimized = false;

    u32 mWidth;
    u32 mHeight;
};
