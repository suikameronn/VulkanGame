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

#define MAX_TEXTURE_COUNT 5
#define CUBEMAP_FACE_COUNT 6
#define CUBEMAP_FACE_FRONT 0
#define CUBEMAP_FACE_BACK 1
#define CUBEMAP_FACE_RIGHT 2
#define CUBEMAP_FACE_LEFT 3
#define CUBEMAP_FACE_TOP 4
#define CUBEMAP_FACE_BOTTOM 5

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

struct ModelDescriptor
{
    VkDescriptorSetLayout layout;
    VkDescriptorSetLayout materialLayout;
    VkDescriptorSetLayout lightLayout;

    VkPipelineLayout texturePipelineLayout;
    VkPipeline texturePipeline;
    
    VkPipelineLayout coliderPipelineLayout;
    VkPipeline coliderPipeline;

    void destroy(VkDevice& device)
    {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
        vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, lightLayout, nullptr);

        vkDestroyPipelineLayout(device, texturePipelineLayout, nullptr);
        vkDestroyPipelineLayout(device, coliderPipelineLayout, nullptr);

        vkDestroyPipeline(device, texturePipeline, nullptr);
        vkDestroyPipeline(device, coliderPipeline, nullptr);
    }
};

struct PointLightUBO
{
    alignas(16)int lightCount;
    alignas(16) std::array<glm::vec4,50> pos;
    alignas(16) std::array<glm::vec4,50> color;
};

struct DirectionalLightUBO
{
    alignas(16) int lightCount;
    alignas(16) std::array<glm::vec4, 50> dir;
    alignas(16) std::array<glm::vec4, 50> color;
};

struct ShadowMapUBO
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct MatricesUBO {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec4 worldCameraPos;
    alignas(16) int lightCount;
    alignas(16) std::array<glm::mat4,20> lightMVP;
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

struct ImageDescriptor
{
    TextureData* emptyTex;
    VkDescriptorSetLayout layout;
    VkDescriptorSet descriptorSet;

    void destroy(VkDevice& device)
    {
        emptyTex->destroy(device);

        vkDestroyDescriptorSetLayout(device, layout, nullptr);
    }
};

struct ShaderBuffer
{
    VkDevice device;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;
    int32_t count = 0;
    void* mapped = nullptr;
};

struct FrameBufferAttachment {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;

    void destory(VkDevice& device)
    {
        vkDestroyImageView(device, view, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }
};

struct OffScreenPass {
    int32_t width, height;
    std::vector<VkFramebuffer> frameBuffer;
    std::vector<FrameBufferAttachment> imageAttachment;
    VkRenderPass renderPass;
    VkSampler sampler;
    VkDescriptorSetLayout layout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    void setFrameCount(int count)
    {
        frameBuffer.resize(count);
        imageAttachment.resize(count);
        descriptorSets.resize(count);
    }

    void destroy(VkDevice& device)
    {
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, layout, nullptr);

        for (auto& framebuffer : frameBuffer)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto& attachment : imageAttachment)
        {
            attachment.destory(device);
        }

        vkDestroySampler(device, sampler, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);
    }
};

struct ShadowMapData
{
    glm::mat4 proj;

    int shadowMapScale;
    OffScreenPass passData;
    std::vector<ShadowMapUBO> matUBOs;
    std::vector<MappedBuffer> mappedBuffers;

    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSet> descriptorSets;

    void setFrameCount(int frameCount)
    {
        matUBOs.resize(frameCount);
        mappedBuffers.resize(frameCount);
        passData.setFrameCount(frameCount);
    }

    void destroy(VkDevice& device)
    {
        passData.destroy(device);

        vkDestroyDescriptorSetLayout(device, layout, nullptr);

        for (auto& buffer : mappedBuffers)
        {
            buffer.destroy(device);
        }
    }
};

struct CubemapData
{
    glm::mat4 view;
    glm::mat4 proj;

    OffScreenPass passData;
    std::vector<MatricesUBO> matUBOs;
    std::vector<MappedBuffer> mappedBuffers;

    TextureData* multiTexture;//キューブマッピング用のテクスチャデータ

    VkDescriptorSetLayout layout;
    VkDescriptorSet descriptorSet;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    CubemapData()
    {
        multiTexture = new TextureData();
    }

    void setFrameCount(int frameCount)
    {
        matUBOs.resize(frameCount);
        mappedBuffers.resize(frameCount);
        passData.setFrameCount(frameCount);
    }

    void destroy(VkDevice& device)
    {
        multiTexture->destroy(device);
        passData.destroy(device);

        vkDestroyPipeline(device, pipeline,nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, layout, nullptr);

        for (auto& buffer : mappedBuffers)
        {
            buffer.destroy(device);
        }
    }
};

class VulkanBase
{
private:

    const float shadowMapTop = -500;
    const float shadowMapBottom = 500;
    const float shadowMapRight = 500;
    const float shadowMapLeft = -500;
    const float shadowMapNear = 0.1f;
    const float shadowMapFar = 1000.0f;

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

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;

    bool firstSendModel = true;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    int descriptorSetCount;

    ModelDescriptor modelDescriptor;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    VkDescriptorPool descriptorPool;

    ImageDescriptor emptyImage;//空のテクスチャ用データ

    CubemapData cubemapData;//キューブマッピング用のデータ

    ShadowMapData shadowMapData;//シャドウマップ用のデータ

    void setUpComputingShader();

    void prepareModelDescLayouts();

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
    void createImageSampler(VkSamplerMipmapMode mipMapMode, VkSamplerAddressMode addressMode
        , VkFilter magFilter, VkFilter minFilter, VkSampler& sampler);
    void createImageViews();
    void createRenderPass();

    void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout);
    void createDescriptorSetLayout(std::shared_ptr<Material> material);

    void createGraphicsPipeline(std::string vertFile, std::string fragFile, VkPrimitiveTopology topology,VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline);
    void createShadowMapPipeline(std::string vertexPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);
    void createCalcCubeMapPipeline(std::string vertexPath,std::string fragPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);

    void createFramebuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    uint32_t calcMipMapLevel(uint32_t width, uint32_t height);

    void createTextureImage(TextureData& textureData,std::shared_ptr<ImageData> image);//デフォルトのテクスチャの画像データの作成
    void createTextureImage();//空のテクスチャを作成
    void createTextureImage(std::shared_ptr<GltfModel> gltfModel);//gltfモデルのマテリアルにテクスチャ用データを作成
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels,uint32_t layerCount);

    void createTextureImageView(TextureData& textureData);//デフォルトのテクスチャのビューを作成
    void createTextureImageView();//空のテクスチャのビューを作成
    void createTextureImageView(std::shared_ptr<GltfModel> gltfModel);//gltfモデルのテクスチャのビューを作成
    
    void createTextureSampler(TextureData* textureData);//デフォルトのテクスチャのサンプラーの作成
    void createTextureSampler();//空のテクスチャのサンプラーの作成
    void createTextureSampler(std::shared_ptr<GltfModel> gltfModel);//gltfモデルのテクスチャのサンプラーの作成
    
    VkImageView createImageView(VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount);//テクスチャのビューを作成
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format
        , VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,uint32_t layerCount);
    void createVertexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    void createVertexBuffer(std::shared_ptr<Colider> colider);
    void createIndexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    void createIndexBuffer(std::shared_ptr<Colider> colider);

    void createUniformBuffers(std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Model> model);
    void createUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Colider> colider);
    void createUniformBuffer(std::shared_ptr<Material> material);
    void createUniformBuffer(int lightCount,MappedBuffer* mappedBuffer,unsigned long long size);

    void createShaderMaterialUBO(std::shared_ptr<Material> material);

    void createDescriptorPool();

    void allocateDescriptorSets(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model);
    void allocateDescriptorSet(VkDescriptorSetLayout& layout, GltfNode* node,std::shared_ptr<Model> model);
    void allocateDescriptorSet(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model);
    void allocateDescriptorSet(std::shared_ptr<Material> material);

    void createDescriptorSets(std::shared_ptr<Model> model);
    void createDescriptorSet(GltfNode* node,std::shared_ptr<Model> model);
    void createDescriptorSet(std::shared_ptr<Model> model);
    void createDescriptorSet(std::shared_ptr<Material> material, std::shared_ptr<GltfModel> gltfModel);
    void createDescriptorSet_CubeMap(GltfNode* node, std::shared_ptr<Model> model);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();

    void setupShadowMapDepth(VkCommandBuffer& commandBuffer);

    void updateUniformBuffers(std::shared_ptr<Model> model);
    void updateUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    void updateUniformBuffer(std::shared_ptr<Colider> colider);
    void updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights, MappedBuffer& mappedBuffer);
    void updateUniformBuffer(std::vector<std::shared_ptr<DirectionalLight>>& directionalLights, MappedBuffer& mappedBuffer);
    void updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights, std::vector<std::shared_ptr<DirectionalLight>>& directionalLights);

    void drawFrame();
    void drawCubeMap(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer);
    void drawMesh(GltfNode* node,std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer);
    void calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer, OffScreenPass& pass);

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
    void createDescriptorInfos(VkPipelineLayout& pLayout, VkPipeline& pipeline, std::shared_ptr<Model> model);
    void createDescriptorInfo(VkPipelineLayout& pLayout, VkPipeline& pipeline, GltfNode* node,std::shared_ptr<Model> model);
    void createDescriptorInfo(std::shared_ptr<Colider> colider);

    void createDescriptorData(MappedBuffer& mappedBuffer,VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet, unsigned long long size, VkShaderStageFlags frag);
    void createDescriptorData_ShadowMap(std::vector<VkDescriptorSet>& descriptorSets, OffScreenPass& pass, VkDescriptorSetLayout& layout);

    void createEmptyImage();

    void setPointLights(std::vector<std::shared_ptr<PointLight>> lights);
    void setDirectionalLights(std::vector<std::shared_ptr<DirectionalLight>> lights);
    void prepareShadowMapping(int lightCount);
    void prepareCubemapTextures();
    void createSamplerCube2D();//6枚のテクスチャを作成
    void drawSamplerCube(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer, int index);

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

    void prepareDescriptorData(int lightCount);

    void setGltfModelData(std::shared_ptr<GltfModel> gltfModel);
    void setModelData(std::shared_ptr<Model> model);
    void setCubeMapModel(std::shared_ptr<Model> cubemap);
    void setLightData(std::vector<std::shared_ptr<PointLight>> pointLights, std::vector<std::shared_ptr<DirectionalLight>> dirLights);
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