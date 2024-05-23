#pragma once
#include<vector>
#include<bitset>
#include<vulkan/vulkan.h>

#include"Meshes.h"
#include"Material.h"

struct BufferObject
{
	VkBuffer vertBuffer;
	VkDeviceMemory vertHandler;

	VkBuffer indeBuffer;
	VkDeviceMemory indeHandler;
};

struct DescriptorInfo
{
	VkDescriptorSetLayout layout;
	VkDescriptorPool pool;
	VkPipelineLayout pLayout;
	VkPipeline pipeline;

	bool operator==(const DescriptorInfo& a) const
	{
		return layout == a.layout;
	}

	bool operator!=(const DescriptorInfo& a) const
	{
		return !(layout == a.layout);
	}
};

struct MappedBuffer
{
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;
};

struct TextureData
{
	uint32_t mipLevel;
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
	VkSampler sampler;
};

class Model
{
private:
	std::bitset<8> layoutBit;

	Meshes* meshes;
	Material* material;

	BufferObject pointBuffer;
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

	BufferObject* getPointBuffer();
	MappedBuffer* getMappedBuffer();
	TextureData* getTextureData();
	std::bitset<8> getLayoutBit();
	VkDescriptorSet* getDescriptorSet();
};