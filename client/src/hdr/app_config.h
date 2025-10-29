#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "hdr/imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <cstdio>

// Forward declarations
struct SDL_Window;

// Vulkan configuration structure
struct VulkanConfig {
    VkAllocationCallbacks* allocator;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t queueFamily;
    VkQueue queue;
    VkPipelineCache pipelineCache;
    VkDescriptorPool descriptorPool;
    ImGui_ImplVulkanH_Window mainWindowData;
    uint32_t minImageCount;
    bool swapChainRebuild;
    
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    VkDebugReportCallbackEXT debugReport;
#endif
};

// Application context structure
struct AppContext {
    SDL_Window* window;
    VulkanConfig vulkan;
    float mainScale;
};

// Initialization functions
bool InitializeSDL();
SDL_Window* CreateAppWindow(float& outScale);
void SetupVulkan(ImVector<const char*> instanceExtensions, VulkanConfig& config);
void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height, VulkanConfig& config);
bool InitializeImGui(SDL_Window* window, VulkanConfig& config, float scale);

// Rendering functions
void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* drawData, VulkanConfig& config);
void FramePresent(ImGui_ImplVulkanH_Window* wd, VulkanConfig& config);

// Cleanup functions
void CleanupImGui();
void CleanupVulkanWindow(VulkanConfig& config);
void CleanupVulkan(VulkanConfig& config);
void CleanupSDL(SDL_Window* window);

// Utility functions
void CheckVkResult(VkResult err);
bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension);

#endif // APP_CONFIG_H