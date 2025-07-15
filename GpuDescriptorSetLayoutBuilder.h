#pragma once

#include<iostream>

#include"VulkanCore.h"

class GpuDescriptorSetLayoutBuilder
{
private:

	//�_���f�o�C�X
	VkDevice device;

	//�v���p�e�B��ۑ�����z��
	std::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>> properties;

public:

	GpuDescriptorSetLayoutBuilder(VkDevice& d);

	//�v���p�e�B�̏�����
	GpuDescriptorSetLayoutBuilder initProperty();

	//�^�C�v�Ǝg���V�F�[�_�̐ݒ�
	GpuDescriptorSetLayoutBuilder setProperty(VkDescriptorType type, VkShaderStageFlagBits stage);
	GpuDescriptorSetLayoutBuilder setProperty(int type, int stage);

	//�o�C���f�B���O�̍쐬
	std::vector<VkDescriptorSetLayoutBinding> Build();
};