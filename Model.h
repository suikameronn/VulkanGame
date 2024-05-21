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
	Meshes* meshes;
	Material* material;

	MappedBuffer mappedBuffer;

	uint32_t pointID;

	uint32_t texID;
	TextureData textureData;

	VkDescriptorSet descriptorSet;//ディスクリプタ関連の個々のModelが持たなければならなければならないのは、これだけ

public:
	Model();
	Model(Meshes* m, Material* material);

	void setMeshes(Meshes* meshes);
	void setImageData(ImageData* image);
	void setMaterial(Material* material);

	Meshes* getMeshes();
	Material* getMaterial();

	void setDescriptorSet(VkDescriptorSet* descriptorSet);

	VkDescriptorSet* getDescriptorSet();

	MappedBuffer* getMappedBuffer();
	TextureData* getTextureData();
	BufferObject* getVertBuffer();
	BufferObject* getIndeBuffer();
};