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

extern GLFWwindow* window;

//VkDescriptorSetの確保できる数
#define MAX_VKDESCRIPTORSET 1000

enum Extension
{
    OBJ,
};

//gpuに求めるキューの構造体
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

//スワップチェーンに求める設定の構造体
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

//オブジェクト描画に必要な設定の構造体
struct ModelDescriptor
{
    VkDescriptorSetLayout layout;//MVP行列用レイアウト
    VkDescriptorSetLayout materialLayout;//マテリアルのデータ用のレイアウト
    VkDescriptorSetLayout lightLayout;//ライト用のレイアウト

    VkPipelineLayout texturePipelineLayout;//通常のオブジェクトレンダリング用のパイプラインレイアウト
    VkPipeline texturePipeline;//実際のパイプライン
    
    VkPipelineLayout coliderPipelineLayout;//コライダーレンダリング用のパイプラインレイアウト
    VkPipeline coliderPipeline;//実際のパイプライン

    void destroy(VkDevice& device)//各種gpu上のデータの破棄
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

//gltfモデルがテクスチャを持たなかったとき用のダミーテクスチャ用データ
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

//シェーダにマテリアルデータを渡すためのバッファーの構造体
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

    //ロード中用のコマンドバッファ
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

class VulkanBase
{
private:

    //コライダーの表示非表示
    const bool coliderDraw = true;

    //キューブマップ用の立方体のモデル
    const std::string cubemapPath = "cubemap.glb";

    //シャドウマップ作成時の平衡投影行列用の範囲の変数
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

    //スワップチェーンのフレームバッファの数を設定
    const int MAX_FRAMES_IN_FLIGHT = 2;

    //Vulkan用のインスタンス
    VkInstance instance;
    //デバッグ用
    VkDebugUtilsMessengerEXT debugMessenger;
    //出力先のウィンドウ
    VkSurfaceKHR surface;

    //gpu:デバイス
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    //gpuへのアクセス用
    VkDevice device;

    //デバイスの制約
    VkPhysicalDeviceLimits deviceLimits;

    //gpuに求める描画用キュー
    VkQueue graphicsQueue;
    //gpuに求めるスワップチェーン用のキュー
    VkQueue presentQueue;

    //マルチスレッド時用の描画用キュー
    VkQueue multiThreadGraphicQueue;
    //マルチスレッド時用のスワップチェーン用のキュー
    VkQueue multiThreadPresentQueue;

    //スワップチェーン
    VkSwapchainKHR swapChain;
    //スワップチェーン用の画像の配列
    std::vector<VkImage> swapChainImages;
    //画像のフォーマット
    VkFormat swapChainImageFormat;
    //画像のサイズ
    VkExtent2D swapChainExtent;
    //画像へのビュー
    std::vector<VkImageView> swapChainImageViews;
    //スワップチェーン用のフレームバッファ
    std::vector<VkFramebuffer> swapChainFramebuffers;

    //通常のレンダリングで使用するためのパス
    VkRenderPass renderPass;
    //コマンドバッファー作成用
    VkCommandPool commandPool;
    //マルチスレッド用コマンドプール
    VkCommandPool multiThreadCommandPool;
    //通常でのレンダリングで使用するピクセルのサンプリングの設定
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;

    bool firstSendModel = true;

    //デプスバッファ用の画像
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    //レンダリング用のカラー画像 これを画面に表示する
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    //gpuで確保可能なdescriptorSetの数を超えないようにカウントする
    int descriptorSetCount;

    //通常のレンダリングとコライダーのレンダリングで使用するデータを持つ
    ModelDescriptor modelDescriptor;
    //UIレンダリング用のデータ
    UIRender uiRender;

    //シャドウマップのVkDescriptorSetLayout
    VkDescriptorSetLayout shadowmapLayout;
    //IBLのVkDescriptorSetLayout
    VkDescriptorSetLayout iblLayout;

    //コマンドバッファーの配列 スワップチェーンが持つ画像の数だけバッファーを持つ
    std::vector<VkCommandBuffer> commandBuffers;

    //そのコマンドバッファが使用可能になるまで待つ
    std::vector<VkSemaphore> imageAvailableSemaphores;
    //そのコマンドバッファでの処理が終わったら信号を送る
    std::vector<VkSemaphore> renderFinishedSemaphores;
    //cpuにgpuからレンダリングの終了を知らせる
    std::vector<VkFence> inFlightFences;
    //マルチスレッド用
    std::vector<VkFence> multiThreadFences;
    //現在のフレームで使用するコマンドバッファなどの番号
    uint32_t currentFrame = 0;
    //現在使用可能なスワップチェーンの画像バッファ
    uint32_t availableSwapChaneImageNumber;

    //descriptorSetはこれから作成する
    VkDescriptorPool descriptorPool;

    ImageDescriptor emptyImage;//ダミーのテクスチャ用データ

    //破棄する予定のgpuのバッファのリスト
    DefferedDestruct defferedDestruct;

    //ゲーム終了時にデータのgpu上のデータをすべて破棄する
    void cleanup();
    //スワップチェーンの破棄
    void cleanupSwapChain();
    //ウィンドウサイズが変わった際のスワップチェーンの画像サイズの変更
    void recreateSwapChain();
    //インスタンスの作成
    void createInstance();
    //デバッグ用
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    //レンダー先の出力先のウィンドウの用意
    void createSurface();
    ///
    VkSampleCountFlagBits getMaxUsableSampleCount();
    //Vulkanで扱うデバイスを選択する
    void pickPhysicalDevice();
    //Vulkan上で扱うデバイスデータの作成
    void createLogicalDevice();
    //スワップチェーンの作成
    void createSwapChain();
    //テクスチャのサンプラーの作成 
    void createImageSampler(VkSamplerMipmapMode mipMapMode, VkSamplerAddressMode addressMode
        , VkFilter magFilter, VkFilter minFilter, VkSampler& sampler);
    //テクスチャのビューの作成
    void createImageViews();
    //レンダーパスの作成
    void createRenderPass();

    //MVP行列とアニメーション行列を含むdescriptorSetLayoutを作成
    void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout);
    //ShaderMaterialを含むdescriptorSetLayoutの作成
    void createDescriptorSetLayout(std::shared_ptr<Material> material);

    //通常のレンダリングで使用するパイプラインの作成
    void createGraphicsPipeline(std::string vertFile, std::string fragFile, VkPrimitiveTopology topology,VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline);
    //シャドウマップ作成用のパイプラインを作成
    void createShadowMapPipeline(std::string vertexPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);
    //HDRI画像からキューブマップ用のパイプラインの作成
    void createCalcCubeMapPipeline(std::string vertexPath,std::string fragPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);
    //キューブマップのレンダリング用のパイプラインの作成
    void createCubemapPipeline(std::string vertexPath, std::string fragPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass);
    //IBLのマップ計算用のパイプラインの作成
    void createCalcIBLMapPipeline(std::string vertShaderPath, std::string fragShaderPath, VkDescriptorSetLayout& prePassLayout
        , VkPipelineLayout& pipelineLayout, uint32_t pipelineNum, VkPipeline* pipeline, uint32_t renderPassNum, VkRenderPass* renderPass);

    //通常のレンダリングで仕様するフレームバッファの作成
    void createFramebuffers();
    //コマンドプールの作成 コマンドバッファーはこれから作る
    void createCommandPool();
    //通常のレンダリングで使用するカラーアタッチメントの作成
    void createColorResources();
    //通常のレンダリングで使用するデプスアタッチメントの作成
    void createDepthResources();
    //gpuがサポートしている画像フォーマットの探索
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);
    //gpuがサポートしているデプスイメージのフォーマットの探索
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    //画像のサイズから作成可能なミップマップレベルの計算
    uint32_t calcMipMapLevel(uint32_t width, uint32_t height);
    
    //複数枚のテクスチャ画像を一つのテクスチャ画像にレイヤーを分けて格納する
    //キューブマップ作成時に実行される
    void copyImageToMultiLayerImage(VkImage* srcImages, uint32_t imageCount, uint32_t width, uint32_t height, VkImage& dstImage);

    //画像からテクスチャ画像の作成
    void createTextureImage(std::shared_ptr<ImageData> image, VkFormat format);//ImageDataからVkImageを作成
    void createTextureImage();//空のテクスチャを作成
    void createTextureImage(std::shared_ptr<GltfModel> gltfModel, VkFormat format);//gltfモデルのマテリアルにテクスチャ用データを作成
    //ミップマップ画像の作成
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels,uint32_t layerCount);

    //テクスチャのビューの作成
    void createTextureImageView(TextureData* textureData, VkFormat format);//デフォルトのテクスチャのビューを作成
    void createTextureImageView();//空のテクスチャのビューを作成
    void createTextureImageView(std::shared_ptr<GltfModel> gltfModel,VkFormat format);//gltfモデルのテクスチャのビューを作成
    
    //テクスチャのサンプラーの作成
    void createTextureSampler(TextureData* textureData);//デフォルトのテクスチャのサンプラーの作成
    void createTextureSampler();//空のテクスチャのサンプラーの作成
    void createTextureSampler(std::shared_ptr<GltfModel> gltfModel);//gltfモデルのテクスチャのサンプラーの作成
    
    //テクスチャのビューの作成
    VkImageView createImageView(VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount);//テクスチャのビューを作成
    //空のテクスチャの作成
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format
        , VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    //テクスチャのレイアウトの変更
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount);
    //バッファーからテクスチャの画像の作成
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,uint32_t layerCount);
    
    //頂点バッファーの作成
    void createVertexBuffer(GltfNode* node, std::shared_ptr<GltfModel> gltfModel);
    //UI用
    void createVertexBuffer(std::shared_ptr<UI> ui);
    //コライダー用
    void createVertexBuffer(std::shared_ptr<Colider> colider);
    
    //インデックスバッファーの作成
    void createIndexBuffer(GltfNode* node, std::shared_ptr<GltfModel> gltfModel);
    //UI用
    void createIndexBuffer(std::shared_ptr<UI> ui);
    //コライダー用
    void createIndexBuffer(std::shared_ptr<Colider> colider);

    void createUniformBuffer(std::shared_ptr<Model> model);
    void createUniformBuffer(GltfNode* node, std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Colider> colider);

    //ShaderMaterialの初期設定とバッファーの用意
    void createShaderMaterialUBO(std::shared_ptr<Material> material);

    //descriptorPoolの作成 descriptorSetはここから作成
    void createDescriptorPool();

    //descriptorSetのバッファーの用意
    //gltfモデル用
    void allocateDescriptorSets(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model);
    void allocateDescriptorSet(VkDescriptorSetLayout& layout, GltfNode* node,std::shared_ptr<Model> model);
    //コライダー用
    void allocateDescriptorSet(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model);
    //ShaderMaterial用
    void allocateDescriptorSet(std::shared_ptr<Material> material);

    //descriptorSetの作成
    //gltfモデルの作成
    void createDescriptorSets(std::shared_ptr<Model> model);
    void createDescriptorSet(GltfNode* node,std::shared_ptr<Model> model);
    //コライダー用
    void createDescriptorSet(std::shared_ptr<Model> model);
    //マテリアル用
    void createDescriptorSet(std::shared_ptr<Material> material, std::shared_ptr<GltfModel> gltfModel);
    //キューブマップ用
    void createDescriptorSet_CubeMap(GltfNode* node, std::shared_ptr<Model> model);

    //バッファの作成
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    //使い捨てのコマンドの記録開始
    VkCommandBuffer beginSingleTimeCommands();
    //コマンドの記録終了
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    //バッファ上のデータのコピー
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    //コマンドバッファーを二つ作成する
    void createCommandBuffers();
    //通常のレンダリングのコマンドを記録していく
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    
    //同期用変数の用意
    void createSyncObjects();

    //シェーダの作成
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

    //gltfモデルの頂点バッファーなどの作成、付随するコライダーの頂点のバッファーも用意
    void createMeshesData(std::shared_ptr<GltfModel> gltfModel);
    void createMeshesData(std::shared_ptr<Colider> colider);
    //gltfモデルの各ノードにパイプラインとそのレイアウトを設定する
    void createDescriptorInfos(VkPipelineLayout& pLayout, VkPipeline& pipeline, std::shared_ptr<Model> model);
    void createDescriptorInfo(VkPipelineLayout& pLayout, VkPipeline& pipeline, GltfNode* node,std::shared_ptr<Model> model);
    //コライダーも同様に設定
    void createDescriptorInfo(std::shared_ptr<Colider> colider);

    //ライトのdescriptorSet関係のデータを作成
    void createDescriptorData(MappedBuffer& mappedBuffer,VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet, unsigned long long size, VkShaderStageFlags frag);
    //シャドウマップ作成用のdescriptorSet関係のデータの作成
    void createDescriptorData_ShadowMap(std::vector<VkDescriptorSet>& descriptorSets, OffScreenPass& pass, VkDescriptorSetLayout& layout);

    //ダミーテクスチャ用のデータを作成
    void createEmptyImage();

    //通常のレンダリングで必要なdescriptorSetのレイアウトをあらかじめ作成する
    void createDescriptorSetLayout();

    //パイプラインをあらかじめ作成
    void createPipelines();

    //シャドウマップの作成
    void calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer, OffScreenPass& pass);
    //UI描画用のパイプラインなどを作成する
    void prepareUIRendering();
    //キューブマップ用のテクスチャを作成するためのデータを用意
    void prepareCubemapTextures(std::shared_ptr<Cubemap> cubemap);
    //キューブマップ用の複数のレイヤーを持つテクスチャデータを作成する
    void createMultiLayerTexture(TextureData* dstTextureData, uint32_t layerCount, uint32_t width, uint32_t height, uint32_t mipLevel, VkFormat format);
    //6枚のテクスチャを作成して、キューブマップを作成
    void createSamplerCube2D(OffScreenPass& passData, std::vector<MappedBuffer>& mappedBuffers, std::shared_ptr<Cubemap> cubemap);
    void createSamplerCube2D(IBLSpecularReflection& iblSpecular, std::vector<MappedBuffer>& mappedBuffers, std::shared_ptr<Cubemap> cubemap);
    //LUTの作成
    void createLUT(IBLSpecularBRDF& iblSpecular, MappedBuffer& mappedBuffer, std::shared_ptr<Cubemap> cubemap);
    //キューブマップを背景としてレンダリング
    void drawSamplerCube(GltfNode* node, std::shared_ptr<Model> model
        , uint32_t width, uint32_t height, VkCommandBuffer& commandBuffer, int index, std::vector<VkDescriptorSet>& descriptorSets
        , VkPipelineLayout& pipelineLayout,VkPipeline& pipeline);

    //IBL用のテクスチャを作成
    void createIBL(IBLDiffuse& diffuse, IBLSpecularReflection& reflection, IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap);
    //IBL作成用のオフスクリーンレンダリングの準備をする
    void prepareIBL(std::string vertShaderPath, std::string fragShaderPath
        ,OffScreenPass& passData,VkFormat format,uint32_t mipmapLevel,std::vector<MappedBuffer>& mappedBuffers,std::shared_ptr<Cubemap> cubemap);
    void prepareIBL(IBLSpecularReflection& iblSPeuclar, std::shared_ptr<Cubemap> cubemap);
    //IBLのdiffuseテクスチャを作成
    void createIBLDiffuse(IBLDiffuse& diffuse, std::shared_ptr<Cubemap> cubemap);
    //IBLのspecularテクスチャを作成
    void createIBLSpecular(IBLSpecularReflection& reflection, IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap);
    //IBLのspecularの鏡面反射のテクスチャを作成
    void createIBLSpecularReflection(IBLSpecularReflection& reflection, std::shared_ptr<Cubemap> cubemap);
    //IBLのspecularのBRDFのテクスチャを作成
    void createIBLSpecularBRDF(IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap);
    //6つの画像を一つの画像にまとめて、SamplerCubeを作る
    void createCubeMapTextureFromImages(uint32_t texSize, uint32_t srcTextureMipmapLevel, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, VkFormat format);
    void createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, std::vector<uint32_t>& mipmapLevelSize, VkFormat format);

    //IBL用のDescriptorSetの用意
    void createIBLDescriptor(TextureData* samplerCube,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);
    void createIBLDescriptor(OffScreenPass& passData,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);

    //シャドウマップのレンダリング
    void renderShadowMap(GltfNode* node, std::shared_ptr<Model> model, ShadowMapData& shadowMapData);
    //キューブマップのレンダリング
    void renderCubemap(GltfNode* node, std::shared_ptr<Cubemap> cubemap);

public:

    static VulkanBase* GetInstance()
    {
        if (!vulkanBase)
        {
            vulkanBase = new VulkanBase();
        }

        return vulkanBase;
    }

    //vulkan上のデバイスデータにアクセス
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

    //vulkanの初期化処理
    void initVulkan();

    VkDevice getDevice()
    {
        return device;
    }

    //gpuの処理が終わるまで待機する
    void gpuWaitIdle()
    {
        vkDeviceWaitIdle(device);
    }

    //各種データに応じたuniform buffer用のバッファーの作成
    void createUniformBuffers(std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<UI> ui);
    //ShaderMaterial用
    void createUniformBuffer(std::shared_ptr<Material> material);
    //ライト用
    void createUniformBuffer(int lightCount, MappedBuffer* mappedBuffer, size_t size);

    //ポイントライトのgpu上のバッファーなどを作成
    void createPointLightBuffer(PointLightBuffer& buffer);
    //平行光源のgpu上のバッファーなどを作成
    void createDirectionalLightBuffer(DirectionalLightBuffer& buffer);

    //シャドウマップ用ンデータを用意する、引数としてシーン上のライトの数だけオフスクリーンレンダリングを行う
    void prepareShadowMapping(int lightCount, ShadowMapData& shadowMap);

    //gltfモデルの持つマテリアル用のデータの作成
    void setGltfModelData(std::shared_ptr<GltfModel> gltfModel);
    //Modelクラスの持つバッファーの作成
    void setModelData(std::shared_ptr<Model> model);
    //キューブマップの作成
    void createCubemap(std::shared_ptr<Cubemap> cubemap);

    float getAspect() { return (float)swapChainExtent.width / (float)swapChainExtent.height; }

    //uiのテクスチャを作成する
    void createTexture(std::shared_ptr<ImageData> image, VkFormat format);
    //uiの頂点バッファなどを用意する
    void setUI(std::shared_ptr<UI> ui);
    //ロード画面の描画
    //void drawLoading();
    //ロード画面の終了
    void stopLoading();

    //UIのテクスチャ変更を反映する
    void changeUITexture(TextureData* textureData,MappedBuffer& mappedBuffer,VkDescriptorSet& descriptorSet);
    //uniform bufferのバッファの作成
    void uiCreateUniformBuffer(MappedBuffer& mappedBuffer);

    //gltfモデルの描画
    void drawMesh(GltfNode* node, std::shared_ptr<Model> model,VkCommandBuffer& commandBuffer
        , std::shared_ptr<Cubemap> cubemap, ShadowMapData& shadowMapData, PointLightBuffer& pointLightBuffer, DirectionalLightBuffer& dirLightBuffer);

    //すべてのレンダリングをする前に実行される
    void renderBegin();
    //すべてのレンダリングが終わったとき実行される
    void renderEnd();

    //シャドウマップのレンダリング開始
    void shadowMapBegin(ShadowMapData& shadowMapData);
    //シャドウマップのレンダリング
    void renderShadowMap(std::shared_ptr<Model> model, ShadowMapData& shadowMapData);
    //シャドウマップのレンダリング終了
    void shadowMapEnd();

    //3DモデルとUIのレンダリングを開始
    void sceneRenderBegin();
    //3Dモデルのレンダリング
    void renderModel(std::shared_ptr<Model> model, std::shared_ptr<Cubemap> cubemap
        , ShadowMapData& shadowMapData, PointLightBuffer& pointLightBuffer, DirectionalLightBuffer& dirLightBuffer);
    //uiのレンダリング
    void renderUI(std::shared_ptr<UI> ui);
    //キューブマップのレンダリング
    void renderCubemap(std::shared_ptr<Cubemap> cubemap);
    //3DモデルとUIのレンダリングを終了
    void sceneRenderEnd();

    //gpuのバッファを破棄リストに追加
    void addDefferedDestructBuffer(BufferObject& pointBuffer);
    void addDefferedDestructBuffer(MappedBuffer& mappedBuffer);
    void addDefferedDestructBuffer(VkBuffer& buffer, VkDeviceMemory& memory);
    //gpuのバッファを破棄
    void cleanupDefferedBuffer();
    void allCleanupDefferedBuffer();
};

//ウィンドウサイズを変えた時に呼び出され、次回フレームレンダリング前に、スワップチェーンの画像サイズをウィンドウに合わせる
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

//gpu上のエラーのメッセージを表示
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

//シェーダの読み込み
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