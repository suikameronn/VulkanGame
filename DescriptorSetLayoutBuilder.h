#pragma once

#include<iostream>

#include"VulkanCore.h"

class DescriptorSetLayoutBuilder
{
private:

	//�_���f�o�C�X
	VkDevice device;

	//�v���p�e�B��ۑ�����z��
	std::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>> properties;

public:

	DescriptorSetLayoutBuilder(VkDevice& d);

	//�v���p�e�B�̏�����
	DescriptorSetLayoutBuilder initProperty();

	//�^�C�v�Ǝg���V�F�[�_�̐ݒ�
	DescriptorSetLayoutBuilder setProperty(VkDescriptorType type, VkShaderStageFlagBits stage);
	DescriptorSetLayoutBuilder setProperty(int type, int stage);

	//�o�C���f�B���O�̍쐬
	std::vector<VkDescriptorSetLayoutBinding> Build();
};