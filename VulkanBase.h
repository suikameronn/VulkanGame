#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <functional>
#include<fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include"Storage.h"

extern GLFWwindow* window;

enum Extension
{
    OBJ,
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct MatricesUBO {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 worldCameraPos;
};

struct AnimationUBO
{
    alignas(16) glm::mat4 matrix;
    alignas(16) std::array<glm::mat4, 128> boneMatrix;
    alignas(16) int boneCount;
};

struct PushConstantObj
{
    glm::mat4 modelMatrix;
};

struct EmptyImage
{
    TextureData* emptyTex;
    DescriptorInfo info;
    VkDescriptorSet descriptorSet;
};

class VulkanBase
{
private:
    static VulkanBase* vulkanBase;

    VulkanBase() {};
    VulkanBase(uint32_t limit) { limitVertexBoneDataSize = limit; };

    const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t limitVertexBoneDataSize;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;

    VkCommandPool commandPool;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    bool firstSendModel = true;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    int descriptorSetCount;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    VkDescriptorPool descriptorPool;

    EmptyImage emptyImage;

    void setUpComputingShader();

    void cleanup();
    void cleanupSwapChain();
    void recreateSwapChain();
    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    void createSurface();
    VkSampleCountFlagBits getMaxUsableSampleCount();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout(PrimitiveTextureCount ptc, VkDescriptorSetLayout& descriptorSetLayout);
    void createGraphicsPipeline(PrimitiveTextureCount ptc, VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline);
    void createFramebuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    uint32_t calcMipMapLevel(uint32_t width, uint32_t height);

    void createTextureImage();
    void createTextureImage(std::shared_ptr<Material> material, std::shared_ptr<GltfModel> gltfModel);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    void createTextureImageView();
    void createTextureImageView(std::shared_ptr<Material> material,std::shared_ptr<GltfModel> gltfModel);
    
    void createTextureSampler();
    void createTextureSampler(std::shared_ptr<Material> material, std::shared_ptr<GltfModel> gltfModel);
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format
        , VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createVertexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    void createVertexBuffer(std::shared_ptr<Colider> colider);
    void createIndexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    void createIndexBuffer(std::shared_ptr<Colider> colider);
    void createUniformBuffers(std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Model> model);
    void createUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Colider> colider);
    void createDescriptorPool();
    void allocateDescriptorSets();
    void allocateDescriptorSets(std::shared_ptr<Model> model);
    void allocateDescriptorSet(GltfNode* node,std::shared_ptr<Model> model);
    void allocateDescriptorSet(std::shared_ptr<Model> model);
    void createDescriptorSets();
    void createDescriptorSets(std::shared_ptr<Model> model);
    void createDescriptorSet(GltfNode* node,std::shared_ptr<Model> model);
    void createDescriptorSet(std::shared_ptr<Model> model);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    void updateUniformBuffers(std::shared_ptr<Model> model);
    void updateUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    void updateUniformBuffer(std::shared_ptr<Colider> colider);
    void drawFrame();
    void drawMesh(GltfNode* node,std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();

    void createMeshesData(std::shared_ptr<Model> model);
    void createTextureDatas(std::shared_ptr<Model> model);
    void createDescriptorInfos(std::shared_ptr<Model> model);
    void createDescriptorInfo(GltfNode* node,std::shared_ptr<Model> model);
    void createDescriptorInfo(std::shared_ptr<Colider> colider);

    void createEmptyImage();

public:

    static VulkanBase* GetInstance()
    {
        if (!vulkanBase)
        {
            vulkanBase = new VulkanBase();
        }

        return vulkanBase;
    }

    VkDevice GetDevice()
    {
        if (device)
        {
            return device;
        }
        else
        {
            throw std::runtime_error("GetDevice: failed");
        }
    }

    uint32_t getLimitVertexBoneDataSize()
    {
        return limitVertexBoneDataSize;
    }

    void FinishVulkanBase()
    {
        delete vulkanBase;
        vulkanBase = nullptr;
    }

    ~VulkanBase()
    {
        cleanup();
    }

    bool framebufferResized = false;

    void initVulkan(uint32_t limit);
    void render();

    VkDevice getDevice()
    {
        return device;
    }

    void setGltfModelData(std::shared_ptr<GltfModel> gltfModel);
    void setModelData(std::shared_ptr<Model> model);
    float getAspect() { return (float)swapChainExtent.width / (float)swapChainExtent.height; }

    void updateColiderVertices_OnlyDebug(std::shared_ptr<Colider> colider);
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}