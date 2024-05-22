<<<<<<< HEAD
#pragma once
#include"Model.h"
#include"vulkan/vulkan.h"

#include<bitset>

//ModelクラスにVulkan特有の変数を増やしたようなクラス
class VkModel
{
private:
	VkDevice device;//デストラクタ使用時に必要

	Model* model;

	VkBuffer vertBuffer = nullptr;
	VkDeviceMemory vertHandler = nullptr;

	VkBuffer indiBuffer = nullptr;
	VkDeviceMemory indiHandler = nullptr;

	UniformBufferObject ubo;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMappedPoint;

	uint32_t mipLevel = 0;
	VkImage image = nullptr;
	VkDeviceMemory memory = nullptr;
	VkImageView view = nullptr;
	VkSampler sampler = nullptr;

	std::bitset<8> layoutBit;//右から、テクスチャ...とフラグを立てていく、
	VkDescriptorSetLayout* layout;
	std::vector<VkDescriptorSet>* descriptorSet;

public:
	VkModel(Model* model);
	~VkModel();

	void setModel(Model* model);

	Meshes* getMeshes();
	Material* getMaterial();
	ImageData* getImageData();

	std::bitset<8> getLayoutBit();
	std::pair<std::bitset<8>, ImageData*> getPairLayoutImage();
	void setDescriptorSetLayout(VkDescriptorSetLayout* layout);
	void setDescriptorSet(std::vector<VkDescriptorSet>* descriptorSet);
};
=======
>>>>>>> VulkanSettingInterface
