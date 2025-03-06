#pragma once

#include "MemoryAllocator.h"
#include "SynchronizationObjects.h"
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Jnrlib.h>
#include <optional>
#include <unordered_set>
#include <vector>

namespace Vulkan
{

struct VulkanRendererInfo
{
    struct LayerInfo
    {
        LayerInfo(std::string const &name, bool mandatory = true)
            : name(name), mandatory(mandatory)
        {
        }
        std::string name;
        bool mandatory = true;
    };

    std::vector<LayerInfo> instanceLayers;
    std::vector<LayerInfo> instanceExtensions;

    std::vector<LayerInfo> deviceLayers;
    std::vector<LayerInfo> deviceExtensions;

    GLFWwindow *window;
};

class Renderer : public Jnrlib::ISingletone<Renderer>
{
    MAKE_SINGLETONE_CAPABLE(Renderer);
    friend class CommandList;
    friend class LayoutTracker;
    friend class RenderPass;

private:
    Renderer(VulkanRendererInfo const &);
    ~Renderer();

public:
    VkDevice GetDevice();
    VmaAllocator GetAllocator();

    VkFormat GetBackbufferFormat();
    VkFormat GetDefaultStencilFormat();
    VkFormat GetDefaultDepthFormat();
    VkExtent2D GetBackbufferExtent();

    u32 AcquireNextImage(GPUSynchronizationObject *);
    VkImageView GetSwapchainImageView(u32 index);
    u32 GetSwapchainImageCount();

public:
    /* Default stuff */
    VkPipelineLayout GetEmptyPipelineLayout();
    VkSampler GetPointSampler();
    VkSampler GetFontSampler();

    VkPipelineCache GetPipelineCache();

public:
    void WaitIdle();
    void OnResize();

private:
    void InitInstance(VulkanRendererInfo const &info);
    void PickPhysicalDevice();
    void PickQueueFamilyIndices();
    void InitDevice(VulkanRendererInfo const &info);
    void InitSurface();
    void InitSwapchain();
    void InitAllocator();

private:
    struct QueueFamilyIndices
    {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;

        bool IsEmpty()
        {
            return !graphicsFamily.has_value() && !presentFamily.has_value();
        }

        bool IsComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }

        std::unordered_set<u32> GetUniqueFamilyIndices()
        {
            std::unordered_set<u32> uniqueFamilyIndices;
            uniqueFamilyIndices.reserve(sizeof(*this) /
                                        sizeof(decltype(graphicsFamily)));

            if (graphicsFamily.has_value())
                uniqueFamilyIndices.insert(*graphicsFamily);
            if (presentFamily.has_value())
                uniqueFamilyIndices.insert(*presentFamily);

            return uniqueFamilyIndices;
        }
    };

    struct ExtensionsOutput
    {
        std::vector<const char *> extensionNames = {};
        std::optional<VkDebugUtilsMessengerCreateInfoEXT> debugUtils = {};
        std::optional<VkPhysicalDeviceDynamicRenderingFeaturesKHR>
            dynamicRendering = {};
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

private:
    std::vector<const char *> GetEnabledInstanceLayers(
        decltype(VulkanRendererInfo::instanceLayers) const &layers);
    ExtensionsOutput HandleEnabledInstanceExtensions(
        decltype(VulkanRendererInfo::instanceExtensions) const &extensions);

    std::vector<const char *> GetEnabledDeviceLayers(
        decltype(VulkanRendererInfo::instanceLayers) const &layers);
    ExtensionsOutput HandleEnabledDeviceExtensions(
        decltype(VulkanRendererInfo::instanceExtensions) const &extensions);

    SwapchainSupportDetails GetSwapchainCapabilities();

    VkPresentModeKHR SelectBestPresentMode(
        std::vector<VkPresentModeKHR> const &presentModes);
    VkSurfaceFormatKHR SelectBestSurfaceFormat(
        std::vector<VkSurfaceFormatKHR> const &formats);
    VkExtent2D SelectSwapchainExtent(
        VkSurfaceCapabilitiesKHR const &capabilities);

private:
    GLFWwindow *mWindow;

    bool mSupportsDynamicRendering = false;

    VkInstance mInstance;
    std::vector<const char *> mInstanceLayers;
    ExtensionsOutput mInstanceExtensions;
    VkDebugUtilsMessengerEXT mDebugUtilsMessenger;

    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceProperties mPhysicalDeviceProperties;
    VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;
    QueueFamilyIndices mQueueIndices;

    VkDevice mDevice;
    std::vector<const char *> mDeviceLayers;
    ExtensionsOutput mDeviceExtensions;
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

    VkSurfaceKHR mRenderingSurface;
    VkFormat mSwapchainFormat;
    VkExtent2D mSwapchainExtent;
    SwapchainSupportDetails mSwapchainDetails;
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    std::vector<VkImageLayout>
        mSwapchainImageLayouts; // Used in command buffer to figure out what to
                                // do with transitions

    VmaAllocator mAllocator;

    /* Some "default" variables will be store in the renderer, so they will be
     * available as soon as rendering is needed and will be released as soon as
     * rendering is not needed
     */
    VkPipelineLayout mEmptyPipelineLayout = VK_NULL_HANDLE;
    VkSampler mPointSampler = VK_NULL_HANDLE;
    VkSampler mFontSampler = VK_NULL_HANDLE;
    VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
};
} // namespace Vulkan
