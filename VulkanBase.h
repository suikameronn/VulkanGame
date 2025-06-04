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

#include<vulkan/vulkan.h>

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include"Storage.h"
#include"Cubemap.h"
#include"Scene.h"
#include"Text.h"

extern GLFWwindow* window;

//VkDescriptorSet�̊m�ۂł��鐔
#define MAX_VKDESCRIPTORSET 1000

//gpu��̃G���[�̃��b�Z�[�W��\��
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

//�V�F�[�_�̓ǂݍ���
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

enum Extension
{
    OBJ,
};

//gpu�ɋ��߂�L���[�̍\����
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

//�X���b�v�`�F�[���ɋ��߂�ݒ�̍\����
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

//�I�u�W�F�N�g�`��ɕK�v�Ȑݒ�̍\����
struct ModelDescriptor
{
    VkDescriptorSetLayout layout;//MVP�s��p���C�A�E�g
    VkDescriptorSetLayout materialLayout;//�}�e���A���̃f�[�^�p�̃��C�A�E�g
    VkDescriptorSetLayout lightLayout;//���C�g�p�̃��C�A�E�g
    VkDescriptorSetLayout raycastLayout;//���_�o�b�t�@���������C�L���X�g�p�̃��C�A�E�g

    VkPipelineLayout texturePipelineLayout;//�ʏ�̃I�u�W�F�N�g�����_�����O�p�̃p�C�v���C�����C�A�E�g
    VkPipeline texturePipeline;//���ۂ̃p�C�v���C��
    
    VkPipelineLayout coliderPipelineLayout;//�R���C�_�[�����_�����O�p�̃p�C�v���C�����C�A�E�g
    VkPipeline coliderPipeline;//���ۂ̃p�C�v���C��

    void destroy(VkDevice& device)//�e��gpu��̃f�[�^�̔j��
    {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
        vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, lightLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, raycastLayout, nullptr);

        vkDestroyPipelineLayout(device, texturePipelineLayout, nullptr);
        vkDestroyPipelineLayout(device, coliderPipelineLayout, nullptr);

        vkDestroyPipeline(device, texturePipeline, nullptr);
        vkDestroyPipeline(device, coliderPipeline, nullptr);
    }
};

//gltf���f�����e�N�X�`���������Ȃ������Ƃ��p�̃_�~�[�e�N�X�`���p�f�[�^
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

//�V�F�[�_�Ƀ}�e���A���f�[�^��n�����߂̃o�b�t�@�[�̍\����
struct ShaderBuffer
{
    VkDevice device;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;
    int32_t count = 0;
    void* mapped = nullptr;
};

struct SpecularPushConstant
{
    float roughness;
};

struct UIRender
{
    std::string vertPath;
    std::string fragPath;

    VkDescriptorSetLayout layout;
    VkPipelineLayout pLayout;
    VkPipeline pipeline;

    //���[�h���p�̃R�}���h�o�b�t�@
    std::array<VkCommandBuffer,2> loadCommandBuffers;

    void destroy(VkDevice& device,VkCommandPool& commandPool)
    {
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, layout, nullptr);

        vkFreeCommandBuffers(device, commandPool, 1, loadCommandBuffers.data());
    }
};

struct DefferedDestruct
{
    std::vector<std::list<VkBuffer>> bufferList;
    std::vector<std::list<VkDeviceMemory>> memoryList;

    void setSwapChainImageCount(uint32_t count)
    {
        bufferList.resize(count);
        memoryList.resize(count);
    }
};

struct alignas(16) RaycastReturn
{
    int hitCount;

    std::array<uintptr_t,10> pointer;
    std::array<float,10> distance;

    void initilize()
    {
        hitCount = -1;
        std::fill(pointer.begin(), pointer.end(), 0);
        std::fill(distance.begin(), distance.end(), -1.0f);
    }
};

struct RaycastPushConstant
{
    uint32_t indexCount;
    uint64_t pointer;
};

struct Raycast
{
    //���C�L���X�g�p�̃R���s���[�g�V�F�[�_
    const std::string shaderPath = "shaders/raycast.comp.spv";
    //���C�L���X�g�p�̃p�C�v���C���ƃ��C�A�E�g
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    //���C�L���X�g�p�̃f�B�X�N���v�^�Z�b�g�̃��C�A�E�g
    VkDescriptorSetLayout layout;
    //���C�L���X�g�p�̃f�B�X�N���v�^�Z�b�g
    VkDescriptorSet descriptorSet;
    //���C�L���X�g�p�̃t�F���X
    VkFence fence;

    //�X�g���[�W�o�b�t�@�̓��e�����Z�b�g���邽�߂̈ȍ~�p�o�b�t�@
    MappedBuffer stagingBuffer;

    //���C�����������Ƃ��ɕԂ��f�[�^���L�^����o�b�t�@
    MappedBuffer storage;

    //���C�L���X�g�p�̃R�}���h�o�b�t�@
    VkCommandBuffer commandBuffer;

    void destroy(VkDevice& device, VkCommandPool& commandPool)
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
        vkDestroyFence(device, fence, nullptr);

        //storage.destroy(device);
        //stagingBuffer.destroy(device);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void setupPipelineLayout(VkDevice& device,ModelDescriptor& modelDescriptor)
    {
        VkPushConstantRange pushConstant;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(RaycastPushConstant);
        pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        std::array<VkDescriptorSetLayout, 3> layouts;
        layouts[0] = layout;
        layouts[1] = modelDescriptor.layout;
        layouts[2] = modelDescriptor.raycastLayout;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline layout!");
        }
    }

    void setupPipeline(VkDevice& device)
    {
        auto computeShaderCode = readFile(shaderPath);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = computeShaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(computeShaderCode.data());

        VkShaderModule computeShaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
        computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderStageInfo.module = computeShaderModule;
        computeShaderStageInfo.pName = "main";

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.stage = computeShaderStageInfo;

        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline!");
        }

        vkDestroyShaderModule(device, computeShaderModule, nullptr);
    }

    void setupLayout(VkDevice& device)
    {
        std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings{};
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].pImmutableSamplers = nullptr;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[1].pImmutableSamplers = nullptr;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute descriptor set layout!");
        }
    }

    void setupDescriptorSet(VkDevice& device,VkDescriptorPool& descriptorPool)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
        allocInfo.pSetLayouts = &layout;

        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    void setupFence(VkDevice& device)
    {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create compute synchronization objects for a frame!");
        }
    }

    void waitFence(VkDevice& device)
    {
        vkWaitForFences(device, 1, &fence, true, UINT64_MAX);
        vkResetFences(device, 1, &fence);
    }

    void setupCommandBuffer(VkDevice& device, VkCommandPool& commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    }

    //��x�������s�����C�L���X�g�̗p��
    void setup(VkDevice& device, VkCommandPool& commandPool, VkDescriptorPool& descriptorPool, ModelDescriptor& modelDescriptor)
    {
        setupLayout(device);
        setupDescriptorSet(device, descriptorPool);
        setupPipelineLayout(device, modelDescriptor);
        setupPipeline(device);
        setupFence(device);
        setupCommandBuffer(device, commandPool);
    }

    //���C�L���X�g�J�n���ƂɎ��s
    void startRaycast(Ray& ray,VkDevice& device,VkCommandBuffer& commandBuffer)
    {
        RaycastReturn returnObj{};
        returnObj.initilize();

        vkMapMemory(device, stagingBuffer.uniformBufferMemory, 0, sizeof(RaycastReturn), 0, &stagingBuffer.uniformBufferMapped);
        memcpy(stagingBuffer.uniformBufferMapped, &returnObj, sizeof(RaycastReturn));
        vkUnmapMemory(device, stagingBuffer.uniformBufferMemory);

        VkBufferCopy copyRegion{};
        copyRegion.size = sizeof(RaycastReturn);
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.uniformBuffer, storage.uniformBuffer, 1, &copyRegion);

        vkResetFences(device, 1, &fence);

        //���C�̐��l��o�b�t�@�̒l���X�V�A������
        updateRaycastBuffer(device, ray);
    }

    void updateRaycastBuffer(VkDevice& device,Ray& ray)
    {
        VkDescriptorBufferInfo info{};
        info.buffer = ray.mappedBuffer.uniformBuffer;
        info.offset = 0;
        info.range = ray.getSize();

        VkDescriptorBufferInfo storageInfo{};
        storageInfo.buffer = storage.uniformBuffer;
        storageInfo.offset = 0;
        storageInfo.range = sizeof(RaycastReturn);

        std::vector<VkWriteDescriptorSet> descriptorWrites(2);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &info;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &storageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    //�X�g���[�W�o�b�t�@����l�����o��
    void getStorageBufferData(VkDevice& device, VkCommandBuffer& commandBuffer, RaycastReturn& returnObj)
    {
        //�X�g���[�W�o�b�t�@����J�ڗp�̃o�b�t�@�ɒl���R�s�[����
        VkBufferCopy copyRegion{};
        copyRegion.size = sizeof(RaycastReturn);
        vkCmdCopyBuffer(commandBuffer, storage.uniformBuffer, stagingBuffer.uniformBuffer, 1, &copyRegion);
    }
};

class VulkanBase
{
private:

    //�R���C�_�[�̕\����\��
    const bool coliderDraw = true;

    //�L���[�u�}�b�v�p�̗����̂̃��f��
    const std::string cubemapPath = "cubemap.glb";

    //�V���h�E�}�b�v�쐬���̕��t���e�s��p�͈̔͂̕ϐ�
    const float shadowMapTop = -500;
    const float shadowMapBottom = 500;
    const float shadowMapRight = 500;
    const float shadowMapLeft = -500;
    const float shadowMapNear = 0.1f;
    const float shadowMapFar = 1000.0f;

    static VulkanBase* vulkanBase;

    VulkanBase() {};

    const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    //�X���b�v�`�F�[���̃t���[���o�b�t�@�̐���ݒ�
    const int MAX_FRAMES_IN_FLIGHT = 2;

    //Vulkan�p�̃C���X�^���X
    VkInstance instance;
    //�f�o�b�O�p
    VkDebugUtilsMessengerEXT debugMessenger;
    //�o�͐�̃E�B���h�E
    VkSurfaceKHR surface;

    //gpu:�f�o�C�X
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    //gpu�ւ̃A�N�Z�X�p
    VkDevice device;

    //�f�o�C�X�̐���
    VkPhysicalDeviceLimits deviceLimits;

    //gpu�ɋ��߂�`��p�L���[
    VkQueue graphicsQueue;
    //gpu�ɋ��߂�X���b�v�`�F�[���p�̃L���[
    VkQueue presentQueue;

    //�}���`�X���b�h���p�̕`��p�L���[
    VkQueue multiThreadGraphicQueue;
    //�}���`�X���b�h���p�̃X���b�v�`�F�[���p�̃L���[
    VkQueue multiThreadPresentQueue;

    //�X���b�v�`�F�[��
    VkSwapchainKHR swapChain;
    //�X���b�v�`�F�[���p�̉摜�̔z��
    std::vector<VkImage> swapChainImages;
    //�摜�̃t�H�[�}�b�g
    VkFormat swapChainImageFormat;
    //�摜�̃T�C�Y
    VkExtent2D swapChainExtent;
    //�摜�ւ̃r���[
    std::vector<VkImageView> swapChainImageViews;
    //�X���b�v�`�F�[���p�̃t���[���o�b�t�@
    std::vector<VkFramebuffer> swapChainFramebuffers;

    //�ʏ�̃����_�����O�Ŏg�p���邽�߂̃p�X
    VkRenderPass renderPass;
    //�R�}���h�o�b�t�@�[�쐬�p
    VkCommandPool commandPool;
    //�}���`�X���b�h�p�R�}���h�v�[��
    VkCommandPool multiThreadCommandPool;
    //�ʏ�ł̃����_�����O�Ŏg�p����s�N�Z���̃T���v�����O�̐ݒ�
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;

    bool firstSendModel = true;

    //�f�v�X�o�b�t�@�p�̉摜
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    //�����_�����O�p�̃J���[�摜 �������ʂɕ\������
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    //gpu�Ŋm�ۉ\��descriptorSet�̐��𒴂��Ȃ��悤�ɃJ�E���g����
    int descriptorSetCount;

    //�ʏ�̃����_�����O�ƃR���C�_�[�̃����_�����O�Ŏg�p����f�[�^������
    ModelDescriptor modelDescriptor;
    //UI�����_�����O�p�̃f�[�^
    UIRender uiRender;

    //�V���h�E�}�b�v��VkDescriptorSetLayout
    VkDescriptorSetLayout shadowmapLayout;
    //IBL��VkDescriptorSetLayout
    VkDescriptorSetLayout iblLayout;
    //�t�H���g�p��VkDescriptorSetLayout
    VkDescriptorSetLayout fontLayout;
    //�e�L�X�g�p��VkDescriptorSetLayout
    VkDescriptorSetLayout textLayout;

    //�R�}���h�o�b�t�@�[�̔z�� �X���b�v�`�F�[�������摜�̐������o�b�t�@�[������
    std::vector<VkCommandBuffer> commandBuffers;

    //���̃R�}���h�o�b�t�@���g�p�\�ɂȂ�܂ő҂�
    std::vector<VkSemaphore> imageAvailableSemaphores;
    //���̃R�}���h�o�b�t�@�ł̏������I�������M���𑗂�
    std::vector<VkSemaphore> renderFinishedSemaphores;
    //cpu��gpu���烌���_�����O�̏I����m�点��
    std::vector<VkFence> inFlightFences;
    //�}���`�X���b�h�p
    std::vector<VkFence> multiThreadFences;
    //���݂̃t���[���Ŏg�p����R�}���h�o�b�t�@�Ȃǂ̔ԍ�
    uint32_t currentFrame = 0;
    //���ݎg�p�\�ȃX���b�v�`�F�[���̉摜�o�b�t�@
    uint32_t availableSwapChaneImageNumber;

    //descriptorSet�͂��ꂩ��쐬����
    VkDescriptorPool descriptorPool;

    ImageDescriptor emptyImage;//�_�~�[�̃e�N�X�`���p�f�[�^

    //�j������\���gpu�̃o�b�t�@�̃��X�g
    DefferedDestruct defferedDestruct;

    //���C�L���X�g�p�̃f�[�^
    Raycast raycast;

    //�Q�[���I�����Ƀf�[�^��gpu��̃f�[�^�����ׂĔj������
    void cleanup();
    //�X���b�v�`�F�[���̔j��
    void cleanupSwapChain();
    //�E�B���h�E�T�C�Y���ς�����ۂ̃X���b�v�`�F�[���̉摜�T�C�Y�̕ύX
    void recreateSwapChain();
    //�C���X�^���X�̍쐬
    void createInstance();
    //�f�o�b�O�p
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    //�����_�[��̏o�͐�̃E�B���h�E�̗p��
    void createSurface();
    ///
    VkSampleCountFlagBits getMaxUsableSampleCount();
    //Vulkan�ň����f�o�C�X��I������
    void pickPhysicalDevice();
    //Vulkan��ň����f�o�C�X�f�[�^�̍쐬
    void createLogicalDevice();
    //�X���b�v�`�F�[���̍쐬
    void createSwapChain();
    //�e�N�X�`���̃T���v���[�̍쐬 
    void createImageSampler(VkSamplerMipmapMode mipMapMode, VkSamplerAddressMode addressMode
        , VkFilter magFilter, VkFilter minFilter, VkSampler& sampler);
    //�e�N�X�`���̃r���[�̍쐬
    void createImageViews();
    //�����_�[�p�X�̍쐬
    void createRenderPass();

    //MVP�s��ƃA�j���[�V�����s����܂�descriptorSetLayout���쐬
    void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout);
    //ShaderMaterial���܂�descriptorSetLayout�̍쐬
    void createDescriptorSetLayout(std::shared_ptr<Material> material);

    //�ʏ�̃����_�����O�Ŏg�p����p�C�v���C���̍쐬
    void createGraphicsPipeline(std::string vertFile, std::string fragFile, VkPrimitiveTopology topology,VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline);
    //�V���h�E�}�b�v�쐬�p�̃p�C�v���C�����쐬
    void createShadowMapPipeline(std::string vertexPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);
    //HDRI�摜����L���[�u�}�b�v�p�̃p�C�v���C���̍쐬
    void createCalcCubeMapPipeline(std::string vertexPath,std::string fragPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);
    //�L���[�u�}�b�v�̃����_�����O�p�̃p�C�v���C���̍쐬
    void createCubemapPipeline(std::string vertexPath, std::string fragPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);
    //IBL�̃}�b�v�v�Z�p�̃p�C�v���C���̍쐬
    void createCalcIBLMapPipeline(std::string vertShaderPath, std::string fragShaderPath, VkDescriptorSetLayout& prePassLayout
        , VkPipelineLayout& pipelineLayout, uint32_t pipelineNum, VkPipeline* pipeline, uint32_t renderPassNum, VkRenderPass* renderPass);

    //�ʏ�̃����_�����O�Ŏd�l����t���[���o�b�t�@�̍쐬
    void createFramebuffers();
    //�R�}���h�v�[���̍쐬 �R�}���h�o�b�t�@�[�͂��ꂩ����
    void createCommandPool();
    //�ʏ�̃����_�����O�Ŏg�p����J���[�A�^�b�`�����g�̍쐬
    void createColorResources();
    //�ʏ�̃����_�����O�Ŏg�p����f�v�X�A�^�b�`�����g�̍쐬
    void createDepthResources();
    //gpu���T�|�[�g���Ă���摜�t�H�[�}�b�g�̒T��
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);
    //gpu���T�|�[�g���Ă���f�v�X�C���[�W�̃t�H�[�}�b�g�̒T��
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    //�摜�̃T�C�Y����쐬�\�ȃ~�b�v�}�b�v���x���̌v�Z
    uint32_t calcMipMapLevel(uint32_t width, uint32_t height);
    
    //�������̃e�N�X�`���摜����̃e�N�X�`���摜�Ƀ��C���[�𕪂��Ċi�[����
    //�L���[�u�}�b�v�쐬���Ɏ��s�����
    void copyImageToMultiLayerImage(VkImage* srcImages, uint32_t imageCount, uint32_t width, uint32_t height, VkImage& dstImage);

    //�摜����e�N�X�`���摜�̍쐬
    void createTextureImage(std::shared_ptr<ImageData> image, VkFormat format);//ImageData����VkImage���쐬
    void createTextureImage();//��̃e�N�X�`�����쐬
    void createTextureImage(std::shared_ptr<GltfModel> gltfModel, VkFormat format);//gltf���f���̃}�e���A���Ƀe�N�X�`���p�f�[�^���쐬
    //�~�b�v�}�b�v�摜�̍쐬
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels,uint32_t layerCount);

    //�e�N�X�`���̃r���[�̍쐬
    void createTextureImageView(TextureData* textureData, VkFormat format);//�f�t�H���g�̃e�N�X�`���̃r���[���쐬
    void createTextureImageView();//��̃e�N�X�`���̃r���[���쐬
    void createTextureImageView(std::shared_ptr<GltfModel> gltfModel,VkFormat format);//gltf���f���̃e�N�X�`���̃r���[���쐬
    
    //�e�N�X�`���̃T���v���[�̍쐬
    void createTextureSampler(TextureData* textureData);//�f�t�H���g�̃e�N�X�`���̃T���v���[�̍쐬
    void createTextureSampler();//��̃e�N�X�`���̃T���v���[�̍쐬
    void createTextureSampler(std::shared_ptr<GltfModel> gltfModel);//gltf���f���̃e�N�X�`���̃T���v���[�̍쐬
    
    //�e�N�X�`���̃r���[�̍쐬
    VkImageView createImageView(VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount);//�e�N�X�`���̃r���[���쐬
    //��̃e�N�X�`���̍쐬
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format
        , VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    //�e�N�X�`���̃��C�A�E�g�̕ύX
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount);
    //�o�b�t�@�[����e�N�X�`���̉摜�̍쐬
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,uint32_t layerCount);
    
    //���_�o�b�t�@�[�̍쐬
    void createVertexBuffer(GltfNode* node, std::shared_ptr<GltfModel> gltfModel);
    //UI�p
    void createVertexBuffer(std::shared_ptr<UI> ui);
    //�e�L�X�g�p
    void createVertexBuffer(std::shared_ptr<Text> text);
    //�R���C�_�[�p
    void createVertexBuffer(std::shared_ptr<Colider> colider);
    
    //�C���f�b�N�X�o�b�t�@�[�̍쐬
    void createIndexBuffer(GltfNode* node, std::shared_ptr<GltfModel> gltfModel);
    //UI�p
    void createIndexBuffer(std::shared_ptr<UI> ui);
    //�e�L�X�g�p
    void createIndexBuffer(std::shared_ptr<Text> text);
    //�R���C�_�[�p
    void createIndexBuffer(std::shared_ptr<Colider> colider);

    void createUniformBuffer(std::shared_ptr<Model> model);
    void createUniformBuffer(GltfNode* node, std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Colider> colider);

    //ShaderMaterial�̏����ݒ�ƃo�b�t�@�[�̗p��
    void createShaderMaterialUBO(std::shared_ptr<Material> material);

    //descriptorPool�̍쐬 descriptorSet�͂�������쐬
    void createDescriptorPool();

    //descriptorSet�̃o�b�t�@�[�̗p��
    //gltf���f���p
    void allocateDescriptorSets(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model);
    void allocateDescriptorSet(VkDescriptorSetLayout& layout, GltfNode* node,std::shared_ptr<Model> model);
    //�R���C�_�[�p
    void allocateDescriptorSet(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model);
    //ShaderMaterial�p
    void allocateDescriptorSet(std::shared_ptr<Material> material);

    //descriptorSet�̍쐬
    //gltf���f���̍쐬
    void createDescriptorSets(std::shared_ptr<Model> model);
    void createDescriptorSet(GltfNode* node,std::shared_ptr<Model> model);
    //�R���C�_�[�p
    void createDescriptorSet(std::shared_ptr<Model> model);
    //�}�e���A���p
    void createDescriptorSet(std::shared_ptr<Material> material, std::shared_ptr<GltfModel> gltfModel);
    //�L���[�u�}�b�v�p
    void createDescriptorSet_CubeMap(GltfNode* node, std::shared_ptr<Model> model);

    //gltf���f���p��VkDesciptorSet�̍쐬(���C�L���X�g���g�p)
    void createRaycastDescriptorSet(GltfNode* node, std::shared_ptr<GltfModel> model);

    //�o�b�t�@�̍쐬
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    //�g���̂ẴR�}���h�̋L�^�J�n
    VkCommandBuffer beginSingleTimeCommands();
    //�R�}���h�̋L�^�I��
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    //�o�b�t�@��̃f�[�^�̃R�s�[
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    //�R�}���h�o�b�t�@�[���쐬����
    void createCommandBuffers();

    //�ʏ�̃����_�����O�̃R�}���h���L�^���Ă���
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    
    //�����p�ϐ��̗p��
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();

    //gltf���f���̒��_�o�b�t�@�[�Ȃǂ̍쐬�A�t������R���C�_�[�̒��_�̃o�b�t�@�[���p��
    void createMeshesData(std::shared_ptr<GltfModel> gltfModel);
    void createMeshesData(std::shared_ptr<Colider> colider);
    //gltf���f���̊e�m�[�h�Ƀp�C�v���C���Ƃ��̃��C�A�E�g��ݒ肷��
    void createDescriptorInfos(VkPipelineLayout& pLayout, VkPipeline& pipeline, std::shared_ptr<Model> model);
    void createDescriptorInfo(VkPipelineLayout& pLayout, VkPipeline& pipeline, GltfNode* node,std::shared_ptr<Model> model);
    //�R���C�_�[�����l�ɐݒ�
    void createDescriptorInfo(std::shared_ptr<Colider> colider);

    //���C�g��descriptorSet�֌W�̃f�[�^���쐬
    void createDescriptorData(MappedBuffer& mappedBuffer,VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet, unsigned long long size, VkShaderStageFlags frag);
    //�V���h�E�}�b�v�쐬�p��descriptorSet�֌W�̃f�[�^�̍쐬
    void createDescriptorData_ShadowMap(std::vector<VkDescriptorSet>& descriptorSets, OffScreenPass& pass, VkDescriptorSetLayout& layout);

    //�_�~�[�e�N�X�`���p�̃f�[�^���쐬
    void createEmptyImage();

    //�ʏ�̃����_�����O�ŕK�v��descriptorSet�̃��C�A�E�g�����炩���ߍ쐬����
    void createDescriptorSetLayout();

    //UI�����_�����O�p��VkDescriptorSet�̃o�b�t�@��p��
    void allocateUIDescriptorSet(const VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);

    //�p�C�v���C�������炩���ߍ쐬
    void createPipelines();

    //�V���h�E�}�b�v�̍쐬
    void calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer, OffScreenPass& pass);
    //UI�`��p�̃p�C�v���C���Ȃǂ��쐬����
    void prepareUIRendering();
    //�L���[�u�}�b�v�p�̃e�N�X�`�����쐬���邽�߂̃f�[�^��p��
    void prepareCubemapTextures(std::shared_ptr<Cubemap> cubemap);
    //�L���[�u�}�b�v�p�̕����̃��C���[�����e�N�X�`���f�[�^���쐬����
    void createMultiLayerTexture(TextureData* dstTextureData, uint32_t layerCount, uint32_t width, uint32_t height, uint32_t mipLevel, VkFormat format);
    //6���̃e�N�X�`�����쐬���āA�L���[�u�}�b�v���쐬
    void createSamplerCube2D(OffScreenPass& passData, std::vector<MappedBuffer>& mappedBuffers, std::shared_ptr<Cubemap> cubemap);
    void createSamplerCube2D(IBLSpecularReflection& iblSpecular, std::vector<MappedBuffer>& mappedBuffers, std::shared_ptr<Cubemap> cubemap);
    //LUT�̍쐬
    void createLUT(IBLSpecularBRDF& iblSpecular, MappedBuffer& mappedBuffer, std::shared_ptr<Cubemap> cubemap);
    //�L���[�u�}�b�v��w�i�Ƃ��ă����_�����O
    void drawSamplerCube(GltfNode* node, std::shared_ptr<Model> model
        , uint32_t width, uint32_t height, VkCommandBuffer& commandBuffer, int index, std::vector<VkDescriptorSet>& descriptorSets
        , VkPipelineLayout& pipelineLayout,VkPipeline& pipeline);

    //IBL�p�̃e�N�X�`�����쐬
    void createIBL(IBLDiffuse& diffuse, IBLSpecularReflection& reflection, IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap);
    //IBL�쐬�p�̃I�t�X�N���[�������_�����O�̏���������
    void prepareIBL(std::string vertShaderPath, std::string fragShaderPath
        ,OffScreenPass& passData,VkFormat format,uint32_t mipmapLevel,std::vector<MappedBuffer>& mappedBuffers,std::shared_ptr<Cubemap> cubemap);
    void prepareIBL(IBLSpecularReflection& iblSPeuclar, std::shared_ptr<Cubemap> cubemap);
    //IBL��diffuse�e�N�X�`�����쐬
    void createIBLDiffuse(IBLDiffuse& diffuse, std::shared_ptr<Cubemap> cubemap);
    //IBL��specular�e�N�X�`�����쐬
    void createIBLSpecular(IBLSpecularReflection& reflection, IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap);
    //IBL��specular�̋��ʔ��˂̃e�N�X�`�����쐬
    void createIBLSpecularReflection(IBLSpecularReflection& reflection, std::shared_ptr<Cubemap> cubemap);
    //IBL��specular��BRDF�̃e�N�X�`�����쐬
    void createIBLSpecularBRDF(IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap);
    //6�̉摜����̉摜�ɂ܂Ƃ߂āASamplerCube�����
    void createCubeMapTextureFromImages(uint32_t texSize, uint32_t srcTextureMipmapLevel, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, VkFormat format);
    void createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, std::vector<uint32_t>& mipmapLevelSize, VkFormat format);

    //IBL�p��DescriptorSet�̗p��
    void createIBLDescriptor(TextureData* samplerCube,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);
    void createIBLDescriptor(OffScreenPass& passData,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);

    //�V���h�E�}�b�v�̃����_�����O
    void renderShadowMap(GltfNode* node, std::shared_ptr<Model> model, ShadowMapData& shadowMapData);
    //�L���[�u�}�b�v�̃����_�����O
    void renderCubemap(GltfNode* node, std::shared_ptr<Cubemap> cubemap);

    //���C�L���X�g�̃R���s���[�g�I�u�W�F�N�g�̍쐬
    void setupRaycast();

    //�ċA�I�̃��C�L���X�g���J�n
    void raycasting(VkCommandBuffer& commandBuffer, Ray& ray
        , GltfNode* node, std::shared_ptr<Model> model);

public:

    static VulkanBase* GetInstance()
    {
        if (!vulkanBase)
        {
            vulkanBase = new VulkanBase();
        }

        return vulkanBase;
    }

    //vulkan��̃f�o�C�X�f�[�^�ɃA�N�Z�X
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

    static void FinishVulkanBase()
    {
        if (vulkanBase)
        {
            delete vulkanBase;
        }
        vulkanBase = nullptr;
    }

    ~VulkanBase()
    {
        cleanup();
    }

    bool framebufferResized = false;

    //vulkan�̏���������
    void initVulkan();

    VkDevice getDevice()
    {
        return device;
    }

    //gpu�̏������I���܂őҋ@����
    void gpuWaitIdle()
    {
        vkDeviceWaitIdle(device);
    }

    //�e��f�[�^�ɉ�����uniform buffer�p�̃o�b�t�@�[�̍쐬
    void createUniformBuffers(std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<UI> ui);
    //ShaderMaterial�p
    void createUniformBuffer(std::shared_ptr<Material> material);
    //���C�g�p
    void createUniformBuffer(int lightCount, MappedBuffer* mappedBuffer, size_t size);

    //�ėp�I�ȃ��j�t�H�[���o�b�t�@�̍쐬�p
    void createUniformBuffer(MappedBuffer* mappedBuffer, size_t size);

    //�|�C���g���C�g��gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void createPointLightBuffer(PointLightBuffer& buffer);
    //���s������gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void createDirectionalLightBuffer(DirectionalLightBuffer& buffer);

    //�V���h�E�}�b�v�p���f�[�^��p�ӂ���A�����Ƃ��ăV�[����̃��C�g�̐������I�t�X�N���[�������_�����O���s��
    void prepareShadowMapping(int lightCount, ShadowMapData& shadowMap);

    //gltf���f���̎��}�e���A���p�̃f�[�^�̍쐬
    void setGltfModelData(std::shared_ptr<GltfModel> gltfModel);
    //Model�N���X�̎��o�b�t�@�[�̍쐬
    void setModelData(std::shared_ptr<Model> model);
    //�L���[�u�}�b�v�̍쐬
    void createCubemap(std::shared_ptr<Cubemap> cubemap);

    float getAspect() { return (float)swapChainExtent.width / (float)swapChainExtent.height; }

    //ui�̃e�N�X�`�����쐬����
    void createTexture(std::shared_ptr<ImageData> image, VkFormat format);
    //ui�̒��_�o�b�t�@�Ȃǂ�p�ӂ���
    void setUI(std::shared_ptr<UI> ui);
    //�e�L�X�g�̒��_�o�b�t�@�Ȃǂ�p�ӂ���
    void setText(std::shared_ptr<Text> text);
    //���[�h��ʂ̕`��
    //void drawLoading();
    //���[�h��ʂ̏I��
    void stopLoading();

    //UI�̃e�N�X�`���ύX�𔽉f����
    void createUIDescriptorSet(TextureData* textureData,MappedBuffer& mappedBuffer,VkDescriptorSet& descriptorSet);
    //UI�p�̃e�N�X�`���𒣂�t����|���S���̂��߂̃o�b�t�@��VkDescriptorSet�����
    void createUIDescriptorSet(MappedBuffer& mappedBuffer, VkDescriptorSet& descriptorSet);
    //uniform buffer�̃o�b�t�@�̍쐬
    void uiCreateUniformBuffer(MappedBuffer& mappedBuffer);

    //�t�H���g�p��VkDescriptorSet���쐬����
    void createFontDescriptorSet(std::shared_ptr<ImageData> atlasTexture, VkDescriptorSet& descriptorSet);

    //gltf���f���̕`��
    void drawMesh(GltfNode* node, std::shared_ptr<Model> model,VkCommandBuffer& commandBuffer
        , std::shared_ptr<Cubemap> cubemap, ShadowMapData& shadowMapData, PointLightBuffer& pointLightBuffer, DirectionalLightBuffer& dirLightBuffer);

    //���ׂẴ����_�����O������O�Ɏ��s�����
    void renderBegin();
    //���ׂẴ����_�����O���I������Ƃ����s�����
    void renderEnd();

    //�V���h�E�}�b�v�̃����_�����O�J�n
    void shadowMapBegin(ShadowMapData& shadowMapData);
    //�V���h�E�}�b�v�̃����_�����O
    void renderShadowMap(std::shared_ptr<Model> model, ShadowMapData& shadowMapData);
    //�V���h�E�}�b�v�̃����_�����O�I��
    void shadowMapEnd();

    //3D���f����UI�̃����_�����O���J�n
    void sceneRenderBegin();
    //3D���f���̃����_�����O
    void renderModel(std::shared_ptr<Model> model, std::shared_ptr<Cubemap> cubemap
        , ShadowMapData& shadowMapData, PointLightBuffer& pointLightBuffer, DirectionalLightBuffer& dirLightBuffer);
    //ui�̃����_�����O
    void renderUI(std::shared_ptr<UI> ui);
    //�L���[�u�}�b�v�̃����_�����O
    void renderCubemap(std::shared_ptr<Cubemap> cubemap);
    //3D���f����UI�̃����_�����O���I��
    void sceneRenderEnd();

    //gpu�̃o�b�t�@��j�����X�g�ɒǉ�
    void addDefferedDestructBuffer(BufferObject& pointBuffer);
    void addDefferedDestructBuffer(MappedBuffer& mappedBuffer);
    void addDefferedDestructBuffer(VkBuffer& buffer, VkDeviceMemory& memory);
    //gpu�̃o�b�t�@��j��
    void cleanupDefferedBuffer();
    void allCleanupDefferedBuffer();

    //�V�F�[�_�̍쐬
    VkShaderModule createShaderModule(const std::vector<char>& code);

    //���C�L���X�g�̊J�n
    void startRaycast(Ray& ray, std::shared_ptr<Model> model, float& distance, GltfNode** node);
};

//�E�B���h�E�T�C�Y��ς������ɌĂяo����A����t���[�������_�����O�O�ɁA�X���b�v�`�F�[���̉摜�T�C�Y���E�B���h�E�ɍ��킹��
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}