#pragma once
#include<iostream>
#include<algorithm>

#include"Model.h"

#define MAX_TEXTURE_COUNT 5//gltfモデルが持つ最大のテクスチャ数
#define CUBEMAP_FACE_COUNT 6//キューブマッピング用の面の数
#define CUBEMAP_FACE_RIGHT 0
#define CUBEMAP_FACE_LEFT 1
#define CUBEMAP_FACE_TOP 2
#define CUBEMAP_FACE_BOTTOM 3
#define CUBEMAP_FACE_FRONT 4
#define CUBEMAP_FACE_BACK 5

//IBL用のテクスチャのサイズ
//サイズを512以上にするとクラッシュする
#define IBL_MAP_SIZE 256

//IBLのspecularのBRDFについての構造体
struct IBLSpecularBRDF
{
    //画像サイズ
    uint32_t size;

    //オフスクリーンレンダリング用のデータ
    OffScreenPass passData;

    //specularとspecular用のdescriptorSet
    VkDescriptorSet descriptorSet;

    //specularとspecular用のコンピュートシェーダへのパス
    std::string vertShaderPath, fragShaderPath;

    //バッファの配列
    std::vector<MappedBuffer> mappedBuffers;

    //オフスクリーンレンダリングのフレーム数を設定する
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);

        passData.setFrameCount(frameCount);
    }

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

        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//IBLのspecularの鏡面反射部分用の構造体
struct IBLSpecularReflection
{
    //画像フォーマット
    VkFormat format;

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
    VkPipelineLayout pipelineLayout;
    std::vector<VkPipeline> pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSet descriptorSet;

    //バッファの配列
    std::vector<MappedBuffer> mappedBuffers;

    IBLSpecularReflection()
    {
        multiLayerTexture = new TextureData();
    }

    //オフスクリーンレンダリングのフレーム数を設定する
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);
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

//イメージベースドライティング用構造体
struct IBLDiffuse
{
    //画像フォーマット
    VkFormat format;

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
    std::string vertShaderPath, fragShaderPath;

    //バッファの配列
    std::vector<MappedBuffer> mappedBuffers;

    IBLDiffuse()
    {
        multiLayerTexture = new TextureData();
    }

    //オフスクリーンレンダリングのフレーム数を設定する
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

        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//キューブマップ作成用の構造体
struct BackGroundColor
{
    //画像フォーマット
    VkFormat format;

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

    VkDescriptorSet descriptorSet;

    //キューブマップ作成用のパイプラインレイアウト
    VkPipelineLayout pipelineLayout;
    //実際のパイプライン
    VkPipeline pipeline;

    BackGroundColor()
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

        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        for (auto& buffer : mappedBuffers)
        {
            buffer.destroy(device);
        }
    }
};

class Cubemap : public Model
{
private:

    BackGroundColor backGroundColor;//キューブマッピング用のデータ
    IBLDiffuse iblDiffuse;//IBLのdiffuseについての構造体
    IBLSpecularReflection iblSpecularReflection;//IBLのspecularの鏡面反射についての構造体
    IBLSpecularBRDF iblSpecularBRDF;//IBLのspecularのBRDFについての構造体

    //キューブマッピング用の画像
    std::shared_ptr<ImageData> hdriMap;

public:

    Cubemap() {};

    //hdri画像の設定と取得
    void setHDRIMap(std::shared_ptr<ImageData> image)
    {
        hdriMap = image;
    }

    std::shared_ptr<ImageData> getHDRIMap()
    {
        return hdriMap;
    }

    //背景色用のデータを返す
    BackGroundColor& getBackGroundColor()
    {
        return backGroundColor;
    }

    //Diffuseのデータを返す
    IBLDiffuse& getDiffuse()
    {
        return iblDiffuse;
    }

    //鏡面反射の反射要素のデータを返す
    IBLSpecularReflection& getSpecularReflection()
    {
        return iblSpecularReflection;
    }

    //鏡面反射のBRDFのデータを返す
    IBLSpecularBRDF& getSpecularBRDF()
    {
        return iblSpecularBRDF;
    }

    //ユニフォームバッファの更新
	void updateUniformBuffer() override;

    //フレーム終了時に実行
    void frameEnd() override;

    void cleanupVulkan() override;
};