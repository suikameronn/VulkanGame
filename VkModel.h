#pragma once
#include"Model.h"
#include"VkObject.h"

#include<bitset>

//Model�N���X��Vulkan���L�̕ϐ��𑝂₵���悤�ȃN���X
class VkModel:public VkObject
{
private:
	Model* model;

	VkBuffer vertBuffer = nullptr;
	VkDeviceMemory vertHandler = nullptr;

	VkBuffer indiBuffer = nullptr;
	VkDeviceMemory indiHandler = nullptr;

	uint32_t mipLevel = 0;
	VkImage image = nullptr;
	VkDeviceMemory memory = nullptr;
	VkImageView view = nullptr;
	VkSampler sampler = nullptr;

	VkPipeline* pipeline = nullptr;

public:
	VkModel(Model* model);
	~VkModel();

	void setModel(Model* model);
	void setPipeline(VkPipeline* p);

	Meshes* getMeshes();
	Material* getMaterial();
	ImageData* getImageData();

	VkImageView getImageView();
	VkSampler getSampler();

	//DescriptroSet�̓o�^�ς݂̔���𒲂ׂ邽�߂̊֐�
	//DescriptorSet�̓��C�A�E�g�������ł��A�e�N�X�`�����قȂ�ꍇ�́ADescriptorSet���قȂ���̂��g�p���邽��
	//���C�A�E�g��ImageData�����������Ƃ��̂ݓo�^�ς݂Ƃ���
	std::pair<std::bitset<8>, ImageData*> getPairLayoutImage();
};