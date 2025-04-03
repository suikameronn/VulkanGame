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

#define MAX_TEXTURE_COUNT 5//gltfモデルが持つ最大のテクスチャ数
#define CUBEMAP_FACE_COUNT 6//キューブマッピング用の面の数
#define CUBEMAP_FACE_RIGHT 0
#define CUBEMAP_FACE_LEFT 1
#define CUBEMAP_FACE_TOP 2
#define CUBEMAP_FACE_BOTTOM 3
#define CUBEMAP_FACE_FRONT 4
#define CUBEMAP_FACE_BACK 5

//IBL用のテクスチャのサイズ
#define IBL_MAP_SIZE 512

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

//ポイントライトの構造体 複数のライトを持つ
struct PointLightUBO
{
    alignas(16)int lightCount;
    alignas(16) std::array<glm::vec4,50> pos;
    alignas(16) std::array<glm::vec4,50> color;
};

//平行光源の構造体 複数のライトを持つ
struct DirectionalLightUBO
{
    alignas(16) int lightCount;
    alignas(16) std::array<glm::vec4, 50> dir;
    alignas(16) std::array<glm::vec4, 50> color;
};

//シャドウマップ作成用のuniform buffer
struct ShadowMapUBO
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

//通常のレンダリング用のuniform buffer
struct MatricesUBO {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec4 worldCameraPos;
    alignas(16) int lightCount;
    alignas(16) std::array<glm::mat4,20> lightMVP;//ライトの行列
};

//通常のレンダリングのアニメーション用の行列
struct AnimationUBO
{
    alignas(16) glm::mat4 matrix;
    alignas(16) std::array<glm::mat4, 128> boneMatrix;
    alignas(16) int boneCount;
};

//gltfモデルのローカル空間へ移動するための行列
struct PushConstantObj
{
    glm::mat4 modelMatrix;
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

//フレームバッファとしてのgpu上の画像用のバッファーの構造体
//その画像へのビューも持つ
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

//シャドウマッピングやキューブマッピング用のオフスクリーンレンダリング用の構造体
struct OffScreenPass {
    int32_t width, height;//レンダリングの出力サイズ
    std::vector<VkFramebuffer> frameBuffer;//レンダリングの出力先のバッファー
    std::vector<FrameBufferAttachment> imageAttachment;//レンダリングの出力先を示すもの
    VkRenderPass renderPass;//利用するレンダーパス
    VkSampler sampler;//レンダリング結果へのサンプラー
    VkDescriptorSetLayout layout;//レンダリング用のレイアウト
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    void setFrameCount(int count)//オフスクリーンレンダリングを行うフレーム数の設定
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

//シャドウマップ作成用の構造体
struct ShadowMapData
{
    //平衡投影の行列
    glm::mat4 proj;

    //シャドウマップの解像度の倍率を設定
    int shadowMapScale;
    //オフスクリーンレンダリング用の構造体
    OffScreenPass passData;
    //シャドウマップ作成用の行列の配列
    std::vector<ShadowMapUBO> matUBOs;
    //行列用のバッファの配列
    std::vector<MappedBuffer> mappedBuffers;

    //シャドウマップを通常のレンダリングで使用するためのレイアウト
    VkDescriptorSetLayout layout;
    //シャドウマップを通常のレンダリングで使用するためのデータ
    std::vector<VkDescriptorSet> descriptorSets;

    //シーン上のライトの数だけ作成
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

//IBLのspecularのBRDFについての構造体
struct IBLSpecularBRDF
{
    //画像サイズ
    uint32_t size;

    //オフスクリーンレンダリング用のデータ
    OffScreenPass passData;

    //通常のレンダリングでspecularのマップを利用するためのレイアウト
    VkDescriptorSetLayout mainPassLayout;

    //specularとspecular用のdescriptorSet
    VkDescriptorSet descriptorSet;

    //specularとspecular用のコンピュートシェーダへのパス
    std::string vertShaderPath, fragShaderPath;

    //レンダリングの出力先のサイズを設定
    void setRenderSize(uint32_t s)
    {
        size = s;

        //specularテクスチャのサイズ
        passData.width = s;
        passData.height = s;
    }

    void destroy(VkDevice& device)
    {
        passData.destroy(device);

        vkDestroyDescriptorSetLayout(device, mainPassLayout, nullptr);
    }
};

//IBLのspecularの鏡面反射部分用の構造体
struct IBLSpecularReflection
{
    //画像のサイズ
    uint32_t size;

    //シェーダへのパス
    std::string vertShaderPath, fragShaderPath;

    //specular用の6つのレイヤーのテクスチャーデータ
    TextureData* multiLayerTexture;

    //specular用のミップマップレベルの配列
    uint32_t mipmapLevel;
    std::vector<uint32_t> mipmapLevelSize;

    //IBLのspecular用のレンダーパス
    std::vector<VkRenderPass> renderPass;

    std::vector<VkFramebuffer> frameBuffer;//レンダリングの出力先のバッファー
    std::vector<FrameBufferAttachment> imageAttachment;//レンダリングの出力先を示すもの

    VkSampler sampler;//レンダリング結果へのサンプラー

    VkDescriptorSetLayout prePassLayout;//テクスチャからSpecular用のマップを作成するためのレイアウト
    VkDescriptorSetLayout mainPassLayout;//通常のレンダリング用のレイアウト
    VkPipelineLayout pipelineLayout;
    std::vector<VkPipeline> pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSet descriptorSet;

    IBLSpecularReflection()
    {
        multiLayerTexture = new TextureData();
    }

    void setRenderSize(uint32_t size)
    {
        this->size = size;

        //specularのミップマップレベル数
        mipmapLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(size, size))) + 1);
        multiLayerTexture->mipLevel = mipmapLevel;

        //各ミップマップレベルの画像のサイズを計算する
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
    }
};

//イメージベースドライティング用構造体
struct IBLDiffuse
{
    //画像サイズ
    uint32_t size;

    //ミップマップレベル
    uint32_t mipmapLevel;

    //オフスクリーンレンダリング用のデータ
    OffScreenPass passData;

    //diffuseについてのテクスチャ
    TextureData* multiLayerTexture;

    //通常のレンダリングでDiffuseのマップを利用するためのレイアウト
    VkDescriptorSetLayout mainPassLayout;

    //diffuseとspecular用のdescriptorSet
    VkDescriptorSet descriptorSet;

    //diffuseとspecular用のコンピュートシェーダへのパス
    std::string vertShaderPath,fragShaderPath;

    IBLDiffuse()
    {
        multiLayerTexture = new TextureData();
    }

    void setMipmapLevel(uint32_t level)
    {
        mipmapLevel = level;
        multiLayerTexture->mipLevel = level;
    }

    //レンダリングの出力先のサイズを設定
    void setRenderSize(uint32_t s)
    {
        size = s;

        //diffuseテクスチャのサイズ
        passData.width = s;
        passData.height = s;
    }

    void destroy(VkDevice& device)
    {
        multiLayerTexture->destroy(device);

        passData.destroy(device);

        vkDestroyDescriptorSetLayout(device, mainPassLayout, nullptr);
    }
};

//キューブマップ作成用の構造体
struct CubemapData
{
    //キューブマップ作成時のビュー行列
    glm::mat4 view;
    //キューブマップ作成時のビュー行列 視野角を度数法で90度に設定
    glm::mat4 proj;

    //オフスクリーンレンダリング用のデータ
    OffScreenPass passData;
    //行列の配列
    std::vector<MatricesUBO> matUBOs;
    //バッファの配列
    std::vector<MappedBuffer> mappedBuffers;

    TextureData* srcHdriTexture;//キューブマッピングの元のテクスチャ
    TextureData* multiTexture;//キューブマッピング用のテクスチャデータ

    //通常のレンダリングで利用するためのレイアウト
    VkDescriptorSetLayout layout;
    VkDescriptorSet descriptorSet;

    //キューブマップ作成用のパイプラインレイアウト
    VkPipelineLayout pipelineLayout;
    //実際のパイプライン
    VkPipeline pipeline;

    CubemapData()
    {
        //キューブマップの元となるHDRI画像のテクスチャデータ
        srcHdriTexture = new TextureData();
        //キューブマップとなる6枚の画像を一つのテクスチャデータにまとめたもの
        multiTexture = new TextureData();
    }

    //キューブマップの面の数だけ作成
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

class VulkanBase
{
private:

    //コライダーの表示非表示
    const bool coliderDraw = false;

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

    //gpuに求める描画用キュー
    VkQueue graphicsQueue;
    //gpuに求めるスワップチェーン用のキュー
    VkQueue presentQueue;

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

    //コマンドバッファーの配列 スワップチェーンが持つ画像の数だけバッファーを持つ
    std::vector<VkCommandBuffer> commandBuffers;

    //そのコマンドバッファが使用可能になるまで待つ
    std::vector<VkSemaphore> imageAvailableSemaphores;
    //そのコマンドバッファでの処理が終わったら信号を送る
    std::vector<VkSemaphore> renderFinishedSemaphores;
    //cpuにgpuからレンダリングの終了を知らせる
    std::vector<VkFence> inFlightFences;
    //現在のフレームで使用するコマンドバッファなどの番号
    uint32_t currentFrame = 0;

    //descriptorSetはこれから作成する
    VkDescriptorPool descriptorPool;

    ImageDescriptor emptyImage;//ダミーのテクスチャ用データ

    CubemapData cubemapData;//キューブマッピング用のデータ
    IBLDiffuse iblDiffuse;//IBLのdiffuseについての構造体
    IBLSpecularReflection iblSpecularReflection;//IBLのspecularの鏡面反射についての構造体
    IBLSpecularBRDF iblSpecularBRDF;//IBLのspecularのBRDFについての構造体

    ShadowMapData shadowMapData;//シャドウマップ用のデータ

    //void setUpComputingShader();

    //通常のレンダリングで使う、descriptorSetLayoutを用意する
    void prepareModelDescLayouts();

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
    void createTextureImage(TextureData& textureData, std::shared_ptr<ImageData> image);//ImageDataからVkImageを作成
    void createTextureImage();//空のテクスチャを作成
    void createTextureImage(std::shared_ptr<GltfModel> gltfModel);//gltfモデルのマテリアルにテクスチャ用データを作成
    //ミップマップ画像の作成
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels,uint32_t layerCount);

    //テクスチャのビューの作成
    void createTextureImageView(TextureData& textureData);//デフォルトのテクスチャのビューを作成
    void createTextureImageView();//空のテクスチャのビューを作成
    void createTextureImageView(std::shared_ptr<GltfModel> gltfModel);//gltfモデルのテクスチャのビューを作成
    
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
    void createVertexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    //コライダー用
    void createVertexBuffer(std::shared_ptr<Colider> colider);
    
    //インデックスバッファーの作成
    void createIndexBuffer(GltfNode* node, std::shared_ptr<Model> model);
    //コライダー用
    void createIndexBuffer(std::shared_ptr<Colider> colider);

    //各種データに応じたuniform buffer用のバッファーの作成
    void createUniformBuffers(std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Model> model);
    void createUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    void createUniformBuffer(std::shared_ptr<Colider> colider);
    //ShaderMaterial用
    void createUniformBuffer(std::shared_ptr<Material> material);
    //ライト用
    void createUniformBuffer(int lightCount,MappedBuffer* mappedBuffer,unsigned long long size);

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
    //シャドウマップの用意
    void setupShadowMapDepth(VkCommandBuffer& commandBuffer);

    //レンダリング時にシェーダに渡す行列の更新
    //gltfモデル用
    void updateUniformBuffers(std::shared_ptr<Model> model);
    void updateUniformBuffer(GltfNode* node,std::shared_ptr<Model> model);
    //コライダー用
    void updateUniformBuffer(std::shared_ptr<Colider> colider);
    //ポイントライト用
    void updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights, MappedBuffer& mappedBuffer);
    //平行光源用
    void updateUniformBuffer(std::vector<std::shared_ptr<DirectionalLight>>& directionalLights, MappedBuffer& mappedBuffer);
    //シャドウマップ作成用
    void updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights, std::vector<std::shared_ptr<DirectionalLight>>& directionalLights);
    //キューブマップ作成用
    void updateUniformBuffers_Cubemap(std::shared_ptr<Model> cubemap);

    //通常のレンダリングを開始する
    void drawFrame();
    //キューブマップのレンダリング
    void drawCubeMap(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer);
    //gltfモデルの描画
    void drawMesh(GltfNode* node,std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer);
    //シャドウマップの作成
    void calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer, OffScreenPass& pass);

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
    void createMeshesData(std::shared_ptr<Model> model);
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

    //ポイントライトのgpu上のバッファーなどを作成
    void setPointLights(std::vector<std::shared_ptr<PointLight>> lights);
    //平行光源のgpu上のバッファーなどを作成
    void setDirectionalLights(std::vector<std::shared_ptr<DirectionalLight>> lights);
    //シャドウマップ用ンデータを用意する、引数としてシーン上のライトの数だけオフスクリーンレンダリングを行う
    void prepareShadowMapping(int lightCount);
    //キューブマップ用のテクスチャを作成するためのデータを用意
    void prepareCubemapTextures();
    //キューブマップ用の複数のレイヤーを持つテクスチャデータを作成する
    void createMultiLayerTexture(TextureData* dstTextureData, uint32_t layerCount, uint32_t width, uint32_t height, uint32_t mipLevel);
    //6枚のテクスチャを作成して、キューブマップを作成
    void createSamplerCube2D(OffScreenPass& passData, std::vector<MappedBuffer>& mappedBuffers);
    void createSamplerCube2D(IBLSpecularReflection& iblSpecular, std::vector<MappedBuffer>& mappedBuffers);
    //LUTの作成
    void createLUT(IBLSpecularBRDF& iblSpecular, MappedBuffer& mappedBuffer);
    //キューブマップを背景としてレンダリング
    void drawSamplerCube(GltfNode* node, std::shared_ptr<Model> model
        , uint32_t width, uint32_t height, VkCommandBuffer& commandBuffer, int index, std::vector<VkDescriptorSet>& descriptorSets
        , VkPipelineLayout& pipelineLayout,VkPipeline& pipeline);

    //IBL用のテクスチャを作成
    void createIBL();
    //IBL作成用のオフスクリーンレンダリングの準備をする
    void prepareIBL(std::string vertShaderPath, std::string fragShaderPath,OffScreenPass& passData,VkFormat format);
    void prepareIBL(IBLSpecularReflection& iblSPeuclar);
    //IBLのdiffuseテクスチャを作成
    void createIBLDiffuse();
    //IBLのspecularテクスチャを作成
    void createIBLSpecular();
    //IBLのspecularの鏡面反射のテクスチャを作成
    void createIBLSpecularReflection();
    //IBLのspecularのBRDFのテクスチャを作成
    void createIBLSpecularBRDF();
    //6つの画像を一つの画像にまとめて、SamplerCubeを作る
    void createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment);
    void createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, std::vector<uint32_t>& mipmapLevelSize);

    //IBL用のDescriptorSetの用意
    void createIBLDescriptor(TextureData* samplerCube,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);
    void createIBLDescriptor(OffScreenPass& passData,VkDescriptorSetLayout& layout,VkDescriptorSet& descriptorSet);

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

    //vulkanの初期化処理
    void initVulkan();

    //レンダリングの開始
    void render();

    VkDevice getDevice()
    {
        return device;
    }

    //通常のレンダリングで必要なdescriptorSetのレイアウトをあらかじめ作成する
    void prepareDescriptorSets();

    //各種バッファーなどのデータの作成 lua実行後に行う
    void prepareDescriptorData(int lightCount);

    //gltfモデルの持つマテリアル用のデータの作成
    void setGltfModelData(std::shared_ptr<GltfModel> gltfModel);
    //Modelクラスの持つバッファーの作成
    void setModelData(std::shared_ptr<Model> model);
    //キューブマップの作成
    void setCubeMapModel(std::shared_ptr<Model> cubemap);
    //各種ライトのバッファなどの作成
    void setLightData(std::vector<std::shared_ptr<PointLight>> pointLights, std::vector<std::shared_ptr<DirectionalLight>> dirLights);
    float getAspect() { return (float)swapChainExtent.width / (float)swapChainExtent.height; }

    void updateColiderVertices_OnlyDebug(std::shared_ptr<Colider> colider);
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