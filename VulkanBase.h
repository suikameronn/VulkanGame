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

extern GLFWwindow* window;

#define MAX_TEXTURE_COUNT 5//gltf���f�������ő�̃e�N�X�`����
#define CUBEMAP_FACE_COUNT 6//�L���[�u�}�b�s���O�p�̖ʂ̐�
#define CUBEMAP_FACE_RIGHT 0
#define CUBEMAP_FACE_LEFT 1
#define CUBEMAP_FACE_TOP 2
#define CUBEMAP_FACE_BOTTOM 3
#define CUBEMAP_FACE_FRONT 4
#define CUBEMAP_FACE_BACK 5

//IBL�p�̃e�N�X�`���̃T�C�Y
//�T�C�Y��512�ȏ�ɂ���ƃN���b�V������
#define IBL_MAP_SIZE 256

//VkDescriptorSet�̊m�ۂł��鐔
#define MAX_VKDESCRIPTORSET 1000

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

    VkPipelineLayout texturePipelineLayout;//�ʏ�̃I�u�W�F�N�g�����_�����O�p�̃p�C�v���C�����C�A�E�g
    VkPipeline texturePipeline;//���ۂ̃p�C�v���C��
    
    VkPipelineLayout coliderPipelineLayout;//�R���C�_�[�����_�����O�p�̃p�C�v���C�����C�A�E�g
    VkPipeline coliderPipeline;//���ۂ̃p�C�v���C��

    void destroy(VkDevice& device)//�e��gpu��̃f�[�^�̔j��
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

//�|�C���g���C�g�̍\���� �����̃��C�g������
struct PointLightUBO
{
    alignas(16)int lightCount;
    alignas(16) std::array<glm::vec4,50> pos;
    alignas(16) std::array<glm::vec4,50> color;
};

//���s�����̍\���� �����̃��C�g������
struct DirectionalLightUBO
{
    alignas(16) int lightCount;
    alignas(16) std::array<glm::vec4, 50> dir;
    alignas(16) std::array<glm::vec4, 50> color;
};

//�V���h�E�}�b�v�쐬�p��uniform buffer
struct ShadowMapUBO
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

//�ʏ�̃����_�����O�p��uniform buffer
struct MatricesUBO {
    glm::vec3 scale;//uv���W�����p
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec4 worldCameraPos;
    alignas(16) int lightCount;
    alignas(16) std::array<glm::mat4,20> lightMVP;//���C�g�̍s��
};

//�ʏ�̃����_�����O�̃A�j���[�V�����p�̍s��
struct AnimationUBO
{
    alignas(16) glm::mat4 nodeMatrix;
    alignas(16) glm::mat4 matrix;
    alignas(16) std::array<glm::mat4, 128> boneMatrix;
    alignas(16) int boneCount;
};

//UI�p��2D�s��
struct MatricesUBO2D
{
    glm::mat4 transformMatrix;
    glm::mat4 projection;
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

//�t���[���o�b�t�@�Ƃ��Ă�gpu��̉摜�p�̃o�b�t�@�[�̍\����
//���̉摜�ւ̃r���[������
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

//�V���h�E�}�b�s���O��L���[�u�}�b�s���O�p�̃I�t�X�N���[�������_�����O�p�̍\����
struct OffScreenPass {
    int32_t width, height;//�����_�����O�̏o�̓T�C�Y
    std::vector<VkFramebuffer> frameBuffer;//�����_�����O�̏o�͐�̃o�b�t�@�[
    std::vector<FrameBufferAttachment> imageAttachment;//�����_�����O�̏o�͐����������
    VkRenderPass renderPass;//���p���郌���_�[�p�X
    VkSampler sampler;//�����_�����O���ʂւ̃T���v���[
    VkDescriptorSetLayout layout;//�����_�����O�p�̃��C�A�E�g
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    void setFrameCount(int count)//�I�t�X�N���[�������_�����O���s���t���[�����̐ݒ�
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

//�V���h�E�}�b�v�쐬�p�̍\����
struct ShadowMapData
{
    //���t���e�̍s��
    glm::mat4 proj;

    //�V���h�E�}�b�v�̉𑜓x�̔{����ݒ�
    int shadowMapScale;
    //�I�t�X�N���[�������_�����O�p�̍\����
    OffScreenPass passData;
    //�V���h�E�}�b�v�쐬�p�̍s��̔z��
    std::vector<ShadowMapUBO> matUBOs;
    //�s��p�̃o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    //�V���h�E�}�b�v��ʏ�̃����_�����O�Ŏg�p���邽�߂̃��C�A�E�g
    VkDescriptorSetLayout layout;
    //�V���h�E�}�b�v��ʏ�̃����_�����O�Ŏg�p���邽�߂̃f�[�^
    std::vector<VkDescriptorSet> descriptorSets;

    //�V�[����̃��C�g�̐������쐬
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

//IBL��specular��BRDF�ɂ��Ă̍\����
struct IBLSpecularBRDF
{
    //�摜�T�C�Y
    uint32_t size;

    //�I�t�X�N���[�������_�����O�p�̃f�[�^
    OffScreenPass passData;

    //�ʏ�̃����_�����O��specular�̃}�b�v�𗘗p���邽�߂̃��C�A�E�g
    VkDescriptorSetLayout mainPassLayout;

    //specular��specular�p��descriptorSet
    VkDescriptorSet descriptorSet;

    //specular��specular�p�̃R���s���[�g�V�F�[�_�ւ̃p�X
    std::string vertShaderPath, fragShaderPath;

    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    //�I�t�X�N���[�������_�����O�̃t���[������ݒ肷��
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);

        passData.setFrameCount(frameCount);
    }

    //�����_�����O�̏o�͐�̃T�C�Y��ݒ�
    void setRenderSize(uint32_t s)
    {
        size = s;

        //specular�e�N�X�`���̃T�C�Y
        passData.width = s;
        passData.height = s;
    }

    void destroy(VkDevice& device)
    {
        passData.destroy(device);

        vkDestroyDescriptorSetLayout(device, mainPassLayout, nullptr);


        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//IBL��specular�̋��ʔ��˕����p�̍\����
struct IBLSpecularReflection
{
    //�摜�t�H�[�}�b�g
    VkFormat format;

    //�摜�̃T�C�Y
    uint32_t size;

    //�V�F�[�_�ւ̃p�X
    std::string vertShaderPath, fragShaderPath;

    //specular�p��6�̃��C���[�̃e�N�X�`���[�f�[�^
    TextureData* multiLayerTexture;

    //specular�p�̃~�b�v�}�b�v���x���̔z��
    uint32_t mipmapLevel;
    std::vector<uint32_t> mipmapLevelSize;

    //IBL��specular�p�̃����_�[�p�X
    std::vector<VkRenderPass> renderPass;

    std::vector<VkFramebuffer> frameBuffer;//�����_�����O�̏o�͐�̃o�b�t�@�[
    std::vector<FrameBufferAttachment> imageAttachment;//�����_�����O�̏o�͐����������

    VkSampler sampler;//�����_�����O���ʂւ̃T���v���[

    VkDescriptorSetLayout prePassLayout;//�e�N�X�`������Specular�p�̃}�b�v���쐬���邽�߂̃��C�A�E�g
    VkDescriptorSetLayout mainPassLayout;//�ʏ�̃����_�����O�p�̃��C�A�E�g
    VkPipelineLayout pipelineLayout;
    std::vector<VkPipeline> pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSet descriptorSet;

    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    IBLSpecularReflection()
    {
        multiLayerTexture = new TextureData();
    }

    //�I�t�X�N���[�������_�����O�̃t���[������ݒ肷��
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);
    }

    void setRenderSize(uint32_t size)
    {
        this->size = size;

        //specular�̃~�b�v�}�b�v���x����
        mipmapLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(size, size))) + 1);
        multiLayerTexture->mipLevel = mipmapLevel;

        //�e�~�b�v�}�b�v���x���̉摜�̃T�C�Y���v�Z����
        mipmapLevelSize.resize(mipmapLevel);
        mipmapLevelSize[0] = size;
        for (int i = 1; i < mipmapLevelSize.size(); i++)
        {
            mipmapLevelSize[i] = static_cast<uint32_t>(std::max(1.0, std::floor(size / std::pow(2.0, i))));
        }

        frameBuffer.resize(mipmapLevel * CUBEMAP_FACE_COUNT);
        imageAttachment.resize(mipmapLevel * CUBEMAP_FACE_COUNT);
        
        renderPass.resize(mipmapLevel);
        pipeline.resize(mipmapLevel);

        descriptorSets.resize(CUBEMAP_FACE_COUNT);
    }

    void destroy(VkDevice& device)
    {
        multiLayerTexture->destroy(device);

        for (auto& p : pipeline)
        {
            vkDestroyPipeline(device, p, nullptr);
        }

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, prePassLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, mainPassLayout, nullptr);

        for (auto& framebuffer : frameBuffer)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto& attachment : imageAttachment)
        {
            attachment.destory(device);
        }

        vkDestroySampler(device, sampler, nullptr);

        for (auto& pass : renderPass)
        {
            vkDestroyRenderPass(device, pass, nullptr);
        }


        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//�C���[�W�x�[�X�h���C�e�B���O�p�\����
struct IBLDiffuse
{
    //�摜�t�H�[�}�b�g
    VkFormat format;

    //�摜�T�C�Y
    uint32_t size;

    //�~�b�v�}�b�v���x��
    uint32_t mipmapLevel;

    //�I�t�X�N���[�������_�����O�p�̃f�[�^
    OffScreenPass passData;

    //diffuse�ɂ��Ẵe�N�X�`��
    TextureData* multiLayerTexture;

    //�ʏ�̃����_�����O��Diffuse�̃}�b�v�𗘗p���邽�߂̃��C�A�E�g
    VkDescriptorSetLayout mainPassLayout;

    //diffuse��specular�p��descriptorSet
    VkDescriptorSet descriptorSet;

    //diffuse��specular�p�̃R���s���[�g�V�F�[�_�ւ̃p�X
    std::string vertShaderPath,fragShaderPath;

    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    IBLDiffuse()
    {
        multiLayerTexture = new TextureData();
    }

    //�I�t�X�N���[�������_�����O�̃t���[������ݒ肷��
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);

        passData.setFrameCount(frameCount);
    }

    void setMipmapLevel(uint32_t level)
    {
        mipmapLevel = level;
        multiLayerTexture->mipLevel = level;
    }

    //�����_�����O�̏o�͐�̃T�C�Y��ݒ�
    void setRenderSize(uint32_t s)
    {
        size = s;

        //diffuse�e�N�X�`���̃T�C�Y
        passData.width = s;
        passData.height = s;
    }

    void destroy(VkDevice& device)
    {
        multiLayerTexture->destroy(device);

        passData.destroy(device);

        vkDestroyDescriptorSetLayout(device, mainPassLayout, nullptr);

        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//�L���[�u�}�b�v�쐬�p�̍\����
struct CubemapData
{
    //�摜�t�H�[�}�b�g
    VkFormat format;

    //�L���[�u�}�b�v�쐬���̃r���[�s��
    glm::mat4 view;
    //�L���[�u�}�b�v�쐬���̃r���[�s�� ����p��x���@��90�x�ɐݒ�
    glm::mat4 proj;

    //�I�t�X�N���[�������_�����O�p�̃f�[�^
    OffScreenPass passData;
    //�s��̔z��
    std::vector<MatricesUBO> matUBOs;
    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    TextureData* srcHdriTexture;//�L���[�u�}�b�s���O�̌��̃e�N�X�`��
    TextureData* multiTexture;//�L���[�u�}�b�s���O�p�̃e�N�X�`���f�[�^

    //�ʏ�̃����_�����O�ŗ��p���邽�߂̃��C�A�E�g
    VkDescriptorSetLayout layout;
    VkDescriptorSet descriptorSet;

    //�L���[�u�}�b�v�쐬�p�̃p�C�v���C�����C�A�E�g
    VkPipelineLayout pipelineLayout;
    //���ۂ̃p�C�v���C��
    VkPipeline pipeline;

    CubemapData()
    {
        //�L���[�u�}�b�v�̌��ƂȂ�HDRI�摜�̃e�N�X�`���f�[�^
        srcHdriTexture = new TextureData();
        //�L���[�u�}�b�v�ƂȂ�6���̉摜����̃e�N�X�`���f�[�^�ɂ܂Ƃ߂�����
        multiTexture = new TextureData();
    }

    //�L���[�u�}�b�v�̖ʂ̐������쐬
    void setFrameCount(int frameCount)
    {
        matUBOs.resize(frameCount);
        mappedBuffers.resize(frameCount);
        passData.setFrameCount(frameCount);
    }

    void destroy(VkDevice& device)
    {
        srcHdriTexture->destroy(device);
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

    //descriptorSet�͂��ꂩ��쐬����
    VkDescriptorPool descriptorPool;

    ImageDescriptor emptyImage;//�_�~�[�̃e�N�X�`���p�f�[�^

    CubemapData cubemapData;//�L���[�u�}�b�s���O�p�̃f�[�^
    IBLDiffuse iblDiffuse;//IBL��diffuse�ɂ��Ă̍\����
    IBLSpecularReflection iblSpecularReflection;//IBL��specular�̋��ʔ��˂ɂ��Ă̍\����
    IBLSpecularBRDF iblSpecularBRDF;//IBL��specular��BRDF�ɂ��Ă̍\����

    ShadowMapData shadowMapData;//�V���h�E�}�b�v�p�̃f�[�^

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
    void createTextureImage(TextureData* textureData, std::shared_ptr<ImageData> image, VkFormat format);//ImageData����VkImage���쐬
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
    void createVertexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    //UI�p
    void createVertexBuffer(std::shared_ptr<UI> ui);
    //�R���C�_�[�p
    void createVertexBuffer(std::shared_ptr<Colider> colider);
    
    //�C���f�b�N�X�o�b�t�@�[�̍쐬
    void createIndexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    //UI�p
    void createIndexBuffer(std::shared_ptr<UI> ui);
    //�R���C�_�[�p
    void createIndexBuffer(std::shared_ptr<Colider> colider);

    //�e��f�[�^�ɉ�����uniform buffer�p�̃o�b�t�@�[�̍쐬
    void createUniformBuffers(std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Model> model);
    void createUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Colider> colider);
    void createUniformBuffer(std::shared_ptr<UI> ui);
    //ShaderMaterial�p
    void createUniformBuffer(std::shared_ptr<Material> material);
    //���C�g�p
    void createUniformBuffer(int lightCount,MappedBuffer* mappedBuffer,unsigned long long size);

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
    //�V���h�E�}�b�v�̗p��
    void setupShadowMapDepth(VkCommandBuffer& commandBuffer);

    //�����_�����O���ɃV�F�[�_�ɓn���s��̍X�V
    //gltf���f���p
    void updateUniformBuffers(std::shared_ptr<Model> model);
    void updateUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    //UI�p
    void updateUniformBuffer(std::shared_ptr<UI> ui);
    //�R���C�_�[�p
    void updateUniformBuffer(std::shared_ptr<Colider> colider);
    //�|�C���g���C�g�p
    void updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights, MappedBuffer& mappedBuffer);
    //���s�����p
    void updateUniformBuffer(std::vector<std::shared_ptr<DirectionalLight>>& directionalLights, MappedBuffer& mappedBuffer);
    //�V���h�E�}�b�v�쐬�p
    void updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights, std::vector<std::shared_ptr<DirectionalLight>>& directionalLights);
    //�L���[�u�}�b�v�쐬�p
    void updateUniformBuffers_Cubemap(std::shared_ptr<Model> cubemap);

    //�ʏ�̃����_�����O���J�n����
    void drawFrame();
    //�L���[�u�}�b�v�̃����_�����O
    void drawCubeMap(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer);
    //gltf���f���̕`��
    void drawMesh(GltfNode* node,std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer);
    //�V���h�E�}�b�v�̍쐬
    void calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer, OffScreenPass& pass);

    //�V�F�[�_�̍쐬
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

    //gltf���f���̒��_�o�b�t�@�[�Ȃǂ̍쐬�A�t������R���C�_�[�̒��_�̃o�b�t�@�[���p��
    void createMeshesData(std::shared_ptr<Model> model);
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

    //�|�C���g���C�g��gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void setPointLights(std::vector<std::shared_ptr<PointLight>> lights);
    //���s������gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void setDirectionalLights(std::vector<std::shared_ptr<DirectionalLight>> lights);
    //�V���h�E�}�b�v�p���f�[�^��p�ӂ���A�����Ƃ��ăV�[����̃��C�g�̐������I�t�X�N���[�������_�����O���s��
    void prepareShadowMapping(int lightCount);
    //UI�`��p�̃p�C�v���C���Ȃǂ��쐬����
    void prepareUIRendering();
    //�L���[�u�}�b�v�p�̃e�N�X�`�����쐬���邽�߂̃f�[�^��p��
    void prepareCubemapTextures();
    //�L���[�u�}�b�v�p�̕����̃��C���[�����e�N�X�`���f�[�^���쐬����
    void createMultiLayerTexture(TextureData* dstTextureData, uint32_t layerCount, uint32_t width, uint32_t height, uint32_t mipLevel, VkFormat format);
    //6���̃e�N�X�`�����쐬���āA�L���[�u�}�b�v���쐬
    void createSamplerCube2D(OffScreenPass& passData, std::vector<MappedBuffer>& mappedBuffers);
    void createSamplerCube2D(IBLSpecularReflection& iblSpecular, std::vector<MappedBuffer>& mappedBuffers);
    //LUT�̍쐬
    void createLUT(IBLSpecularBRDF& iblSpecular, MappedBuffer& mappedBuffer);
    //�L���[�u�}�b�v��w�i�Ƃ��ă����_�����O
    void drawSamplerCube(GltfNode* node, std::shared_ptr<Model> model
        , uint32_t width, uint32_t height, VkCommandBuffer& commandBuffer, int index, std::vector<VkDescriptorSet>& descriptorSets
        , VkPipelineLayout& pipelineLayout,VkPipeline& pipeline);

    //IBL�p�̃e�N�X�`�����쐬
    void createIBL();
    //IBL�쐬�p�̃I�t�X�N���[�������_�����O�̏���������
    void prepareIBL(std::string vertShaderPath, std::string fragShaderPath
        ,OffScreenPass& passData,VkFormat format,uint32_t mipmapLevel,std::vector<MappedBuffer>& mappedBuffers);
    void prepareIBL(IBLSpecularReflection& iblSPeuclar);
    //IBL��diffuse�e�N�X�`�����쐬
    void createIBLDiffuse();
    //IBL��specular�e�N�X�`�����쐬
    void createIBLSpecular();
    //IBL��specular�̋��ʔ��˂̃e�N�X�`�����쐬
    void createIBLSpecularReflection();
    //IBL��specular��BRDF�̃e�N�X�`�����쐬
    void createIBLSpecularBRDF();
    //6�̉摜����̉摜�ɂ܂Ƃ߂āASamplerCube�����
    void createCubeMapTextureFromImages(uint32_t texSize, uint32_t srcTextureMipmapLevel, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, VkFormat format);
    void createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, std::vector<uint32_t>& mipmapLevelSize, VkFormat format);

    //IBL�p��DescriptorSet�̗p��
    void createIBLDescriptor(TextureData* samplerCube,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);
    void createIBLDescriptor(OffScreenPass& passData,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);

    //UI�̕`��
    void drawUI(bool beginRenderPass, VkCommandBuffer& commandBuffer,uint32_t imageIndex);
    void drawUI(std::shared_ptr<UI> ui, bool beginRenderPass, VkCommandBuffer& commandBuffer, uint32_t imageIndex);

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

    //�����_�����O�̊J�n
    void render();

    VkDevice getDevice()
    {
        return device;
    }

    //�ʏ�̃����_�����O�ŕK�v��descriptorSet�̃��C�A�E�g�����炩���ߍ쐬����
    void prepareDescriptorSets();

    //�e��o�b�t�@�[�Ȃǂ̃f�[�^�̍쐬 lua���s��ɍs��
    void prepareDescriptorData(int lightCount);

    //gltf���f���̎��}�e���A���p�̃f�[�^�̍쐬
    void setGltfModelData(std::shared_ptr<GltfModel> gltfModel);
    //Model�N���X�̎��o�b�t�@�[�̍쐬
    void setModelData(std::shared_ptr<Model> model);
    //�L���[�u�}�b�v�̍쐬
    void setCubeMapModel(std::shared_ptr<Model> cubemap);
    //�e�탉�C�g�̃o�b�t�@�Ȃǂ̍쐬
    void setLightData(std::vector<std::shared_ptr<PointLight>> pointLights, std::vector<std::shared_ptr<DirectionalLight>> dirLights);
    float getAspect() { return (float)swapChainExtent.width / (float)swapChainExtent.height; }

    void updateColiderVertices_OnlyDebug(std::shared_ptr<Colider> colider);

    //ui�̃e�N�X�`�����쐬����
    void createUITexture(TextureData* texture, std::shared_ptr<ImageData> image);
    //ui�̒��_�o�b�t�@�Ȃǂ�p�ӂ���
    void setUI(std::shared_ptr<UI> ui);
    //���[�h��ʂ̕`��
    void drawLoading();
    //���[�h��ʂ̏I��
    void stopLoading();

    //UI�̃e�N�X�`���ύX�𔽉f����
    void changeUITexture(TextureData* textureData,MappedBuffer& mappedBuffer,VkDescriptorSet& descriptorSet);
    //uniform buffer�̃o�b�t�@�̍쐬
    void uiCreateUniformBuffer(MappedBuffer& mappedBuffer);
};

//�E�B���h�E�T�C�Y��ς������ɌĂяo����A����t���[�������_�����O�O�ɁA�X���b�v�`�F�[���̉摜�T�C�Y���E�B���h�E�ɍ��킹��
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

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