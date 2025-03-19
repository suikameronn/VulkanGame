#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include<iostream>
#include<vector>
#include<algorithm>

#include<vulkan/vulkan.h>
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include"Object.h"

//画像のデータを担うクラス 4チャンネル専用
class ImageData
{
private:
	//画像の幅
	int width;
	//画像の高さ
	int height;
	//画像の元のチャンネル数
	int texChannels;
	//ピクセルの色の一次元配列
	std::vector<unsigned char> pixels;

public:

	//チャンネル数は4に固定
	ImageData(int width, int height,
		int channels,unsigned char* srcPixels)
	{
		this->width = width;
		this->height = height;
		this->texChannels = 4;
		this->pixels.resize(width * height * 4);
		std::fill(this->pixels.begin(), this->pixels.end(), 255);

		if (channels == 4)//画像に透明度のチャンネルがある場合は、そのまま
		{
			this->pixels.assign(srcPixels, srcPixels + ((width * height * channels) - 1));
		}
		else if (channels == 3)//画像に透明度のチャンネルが無い場合は、すべてのピクセルの透明度を255として
							  //強引にチャンネルを4つに gpuでのテクスチャ作成の都合上
		{
			for (int i = 0; i < width * height; i++)
			{
				pixels[i * 4] = srcPixels[i * 3];
				pixels[i * 4 + 1] = srcPixels[i * 3 + 1];
				pixels[i * 4 + 2] = srcPixels[i * 3 + 2];
			}
		}
	}

	int getWidth()
	{
		return this->width;
	}

	int getHeight()
	{
		return this->height;
	}

	int getTexChannels()
	{
		return this->texChannels;
	}

	unsigned char* getPixelsData()
	{
		return pixels.data();
	}
};

//gpu上のテクスチャデータ
struct TextureData
{
	//テクスチャのみっぷレベル
	uint32_t mipLevel;
	//テクスチャの画像データ
	VkImage image;
	VkDeviceMemory memory;
	//画像のビュー
	VkImageView view;
	//画像のサンプラー テクスチャの境界の設定など
	VkSampler sampler;

	void destroy(VkDevice& device)//各種データの破棄
	{
		vkDestroyImageView(device, view, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, memory, nullptr);
		vkDestroySampler(device, sampler, nullptr);

		delete this;
	}
};

//シェーダ上のテクスチャの参照用番号
//マテリアルがそのテクスチャを持たない場合は、-1が記録される
struct TexCoordSets {
	uint8_t baseColor = 0;
	uint8_t metallicRoughness = 0;
	uint8_t specularGlossiness = 0;
	uint8_t normal = 0;
	uint8_t occlusion = 0;
	uint8_t emissive = 0;
};

//シェーダ上でアクセスるためのマテリアル
//テクスチャに対する係数がメイン
struct ShaderMaterial
{
	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;
	glm::vec4 diffuseFactor;
	glm::vec4 specularFactor;
	int colorTextureSet;
	int PhysicalDescriptorTextureSet;
	int normalTextureSet;
	int occlusionTextureSet;
	int emissiveTextureSet;
	float metallicFactor;
	float roughnessFactor;
	float alphaMask;
	float alphaMaskCutoff;
	float emissiveStrength;
};

//gltfモデルのマテリアル
class Material
{
public:

	Material()
	{
		baseColorTextureIndex = -1;
		metallicRoughnessTextureIndex = -1;
		normalTextureIndex = -1;
		occlusionTextureIndex = -1;
		emissiveTextureIndex = -1;
	}

	enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
	AlphaMode alphaMode = ALPHAMODE_OPAQUE;
	//透明度の最大値の設定
	float alphaCutoff = 1.0f;
	//メタリックの初期値
	float metallicFactor = 1.0f;
	//粗さの初期値
	float roughnessFactor = 1.0f;
	//基本色の設定
	glm::vec4 baseColorFactor = glm::vec4(1.0f);
	//発光
	glm::vec4 emissiveFactor = glm::vec4(0.0f);
	//基本色のテクスチャの番号
	int baseColorTextureIndex;
	//メタリックと粗さのテクスチャの番号
	//粗さはg、メタリックはbチャンネルに格納
	int metallicRoughnessTextureIndex;
	//法線用テクスチャ
	int normalTextureIndex;
	//オクルージョン化リング用テクスチャ番号
	int occlusionTextureIndex;
	//発光用テクスチャ番号
	int emissiveTextureIndex;
	bool doubleSided = false;

	//gltfモデル内のマテリアルの番号
	//gltfモデルのメッシュが持つマテリアルを参照する際に使用
	int index = 0;
	bool unlit = false;
	float emissiveStrength = 1.0f;

	//gpu上のマテリアルに関するデータを紐づけるもの
	//レンダリング時に使用
	VkDescriptorSet descriptorSet;
	//テクスチャが従うuvの種類を記録,-1の場合はそのテクスチャを持たないことを示す
	TexCoordSets texCoordSets;

	//テクスチャに対する係数
	ShaderMaterial shaderMaterial;
	//gpu上のテクスチャに対する係数用のバッファ
	MappedBuffer sMaterialMappedBuffer;

	void setupShaderMaterial()
	{
		shaderMaterial.emissiveFactor = emissiveFactor;

		//それぞれのuv座標のインデックスを設定していく -1の場合は、このマテリアルはそのテクスチャを持たない
		shaderMaterial.colorTextureSet = baseColorTextureIndex != -1 ? texCoordSets.baseColor : -1;
		shaderMaterial.normalTextureSet = normalTextureIndex != -1 ? texCoordSets.normal : -1;
		shaderMaterial.occlusionTextureSet = occlusionTextureIndex != -1 ? texCoordSets.occlusion : -1;
		shaderMaterial.emissiveTextureSet = emissiveTextureIndex != -1 ? texCoordSets.emissive : -1;
		shaderMaterial.alphaMask = static_cast<float>(alphaMode == AlphaMode::ALPHAMODE_MASK);
		shaderMaterial.alphaMaskCutoff = alphaCutoff;
		shaderMaterial.emissiveStrength = emissiveStrength;

		shaderMaterial.baseColorFactor = baseColorFactor;
		shaderMaterial.metallicFactor = metallicFactor;
		shaderMaterial.roughnessFactor = roughnessFactor;
		shaderMaterial.PhysicalDescriptorTextureSet = metallicRoughnessTextureIndex != -1 ? texCoordSets.metallicRoughness : -1;
	}
};