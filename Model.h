#pragma once
#include<vector>
#include"VulkanBase.h"

#include"Meshes.h"
#include"Material.h"

struct MappedBuffer
{
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;
};

struct TextureData
{
	uint32_t mipLevel = 0;
	VkImage image = nullptr;
	VkDeviceMemory memory = nullptr;
	VkImageView view = nullptr;
	VkSampler sampler = nullptr;
};

class Model
{
private:
	std::bitset<8> layoutBit;

	Meshes* meshes;
	Material* material;

	MappedBuffer mappedBuffer;

	uint32_t pointID;

	uint32_t texID;
	TextureData textureData;

	DescriptorInfo descriptorInfo;
	VkDescriptorSet descriptorSet;

public:
	Model();
	Model(Meshes* m, Material* material);

	void setMeshes(Meshes* meshes);
	void setImageData(ImageData* image);
	void setMaterial(Material* material);

	Meshes* getMeshes();
	Material* getMaterial();

	void setDescriptorInfo(DescriptorInfo* info);
	void setDescriptorSet(VkDescriptorSet* descriptorSet);

	DescriptorInfo* getDescriptorInfo();

	MappedBuffer* getMappedBuffer();
	TextureData* getTextureData();
	std::bitset<8> getLayoutBit();
	VkDescriptorSet* getDescriptorSet();
};