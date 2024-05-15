#pragma once
#include"Model.h"
#include"vulkan/vulkan.h"

#include<bitset>

//Model�N���X��Vulkan���L�̕ϐ��𑝂₵���悤�ȃN���X
class VkModel
{
private:
	VkDevice device;//�f�X�g���N�^�g�p���ɕK�v

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

	std::bitset<8> layoutBit;//�E����A�e�N�X�`��...�ƃt���O�𗧂ĂĂ����A
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