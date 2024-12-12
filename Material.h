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

class Material
{
private:

	glm::vec3 diffuse;
	glm::vec3 ambient;
	glm::vec3 specular;
	glm::vec3 emissive;
	float shininess;
	float transmissive;

	//テクスチャのもととなる画像データへのポインタ
	int uvIndex;//使用するuv座標セットのインデックス
	std::shared_ptr<ImageData> image = nullptr;
	TextureData* textureData = nullptr;

	uint32_t imageDataCount;

public:

	Material();
	Material(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular
		, glm::vec3* emissive, float* shininess, float* transmissive);

	~Material();

	void setDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	void setAmbient(glm::vec3 ambient) { this->ambient = ambient; }
	void setSpecular(glm::vec3 specular) { this->specular = specular; }
	void setEmissive(glm::vec3 emissive) { this->emissive = emissive; }
	void setShininess(float shininess) { this->shininess = shininess; }
	void setTransmissive(float transmissive) { this->transmissive = transmissive; }

	glm::vec3 getDiffuse() { return diffuse; }
	glm::vec3 getAmbient() { return ambient; }
	glm::vec3 getSpecular() { return specular; }
	glm::vec3 getEmissive() { return emissive; }
	float getShininess() { return shininess; }
	float getTransmissive() { return transmissive; }

	void setImageData(int uvIndex,std::shared_ptr<ImageData> image);
	std::shared_ptr<ImageData> getImageData();
	TextureData* getTextureData() { return textureData; }
	bool hasImageData();
	bool hasTextureData();
	uint32_t getImageDataCount() { return imageDataCount; }
};