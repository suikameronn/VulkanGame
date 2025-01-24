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

class ImageData
{
private:
	uint32_t width;
	uint32_t height;
	uint32_t texChannels;
	std::vector<unsigned char> pixels;

public:

	ImageData(int width, int height,
		int texChannels,unsigned char* pixels)
	{
		this->width = static_cast<uint32_t>(width);
		this->height = static_cast<uint32_t>(height);
		this->pixels.resize(width * height * texChannels);
		this->pixels.assign(pixels, pixels + ((width * height * texChannels) - 1));
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

struct TextureData
{
	uint32_t mipLevel;
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
	VkSampler sampler;

	void destroy(VkDevice& device)
	{
		vkDestroyImageView(device, view, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, memory, nullptr);
		vkDestroySampler(device, sampler, nullptr);

		delete this;
	}
};

struct TexCoordSets {
	uint8_t baseColor = 0;
	uint8_t metallicRoughness = 0;
	uint8_t specularGlossiness = 0;
	uint8_t normal = 0;
	uint8_t occlusion = 0;
	uint8_t emissive = 0;
};

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
	float alphaCutoff = 1.0f;
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;
	glm::vec4 baseColorFactor = glm::vec4(1.0f);
	glm::vec4 emissiveFactor = glm::vec4(0.0f);
	int baseColorTextureIndex;
	int metallicRoughnessTextureIndex;
	int normalTextureIndex;
	int occlusionTextureIndex;
	int emissiveTextureIndex;
	bool doubleSided = false;

	int index = 0;
	bool unlit = false;
	float emissiveStrength = 1.0f;

	VkDescriptorSetLayout layout;
	VkDescriptorSet descriptorSet;
	TexCoordSets texCoordSets;

	ShaderMaterial shaderMaterial;
	MappedBuffer sMaterialMappedBuffer;

	int getTexCount()
	{
		int texCount = 0;

		texCount += baseColorTextureIndex != -1 ? 1 : 0;
		texCount += metallicRoughnessTextureIndex != -1 ? 1 : 0;
		texCount += normalTextureIndex != -1 ? 1 : 0;
		texCount += occlusionTextureIndex != -1 ? 1 : 0;
		texCount += emissiveTextureIndex != -1 ? 1 : 0;

		return texCount;
	}

	void setupShaderMaterial()
	{
		shaderMaterial.emissiveFactor = emissiveFactor;

		//それぞれのuv座標のインデックスを設定していく
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