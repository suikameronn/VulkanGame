#pragma once
#include"Model.h"
#include"vulkan/vulkan.h"

struct BufferObject
{
	VkBuffer buffer;
	VkDeviceMemory handler;
};

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

//GeometryクラスにVulkan特有の変数を増やしたようなクラス
class VkModel
{
private:
	VkDevice device;//デストラクタ使用時に必要

	Model* model;

	MappedBuffer mappedBuffer;

	BufferObject vertBuffer;

	BufferObject indeBuffer;

	TextureData textureData;

	VkDescriptorSetLayout* layout;
	VkPipeline* pipeline;
	VkDescriptorPool* pool;
	VkDescriptorSet* descriptorSet;

public:
	VkModel(Model* model);
	~VkModel();

	void setModel(Model* model);

	void setLayout(VkDescriptorSetLayout* layout);
	void setPipeline(VkPipeline* pipeline);
	void setPool(VkDescriptorPool* pool);
	void setDescriptorSet(VkDescriptorSet* descriptorSet);

	VkDescriptorSetLayout* getLayout();
	VkDescriptorPool* getPool();
	VkDescriptorSet* getDescriptorSet();
	VkPipeline* getPipeline();

	Meshes* getMeshes();
	Material* getMaterial();
	ImageData* getImageData();
	MappedBuffer* getMappedBuffer();
	TextureData* getTextureData();
	BufferObject* getVertBuffer();
	BufferObject* getIndeBuffer();
};