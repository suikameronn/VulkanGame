#pragma once
#include"Model.h"
#include"vulkan/vulkan.h"

//Geometry�N���X��Vulkan���L�̕ϐ��𑝂₵���悤�ȃN���X
class VkModel
{
private:
	VkDevice device;//�f�X�g���N�^�g�p���ɕK�v

	Model* model;

	int texDescriptorID;
	int uniformDescriptorID;

	VkBuffer vertBuffer = nullptr;
	VkDeviceMemory vertHandler = nullptr;

	VkBuffer indiBuffer = nullptr;
	VkDeviceMemory indiHandler = nullptr;

	uint32_t mipLevel = 0;
	VkImage image = nullptr;
	VkDeviceMemory memory = nullptr;
	VkImageView view = nullptr;
	VkSampler sampler = nullptr;

public:
	VkModel(Model* model);
	~VkModel();

	void setModel(Model* model);

	void setTexDescriptorID(uint32_t id);
	void setUniformDescriptorID(uint32_t id);

	Meshes* getMeshes();
	Material* getMaterial();
	ImageData* getImageData();
};