#pragma once

#include<iostream>

#include"VulkanCore.h"

class DescriptorSetLayoutBuilder : public std::enable_shared_from_this<DescriptorSetLayoutBuilder>
{
private:

	//�v���p�e�B��ۑ�����z��
	std::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>> properties;

public:

	DescriptorSetLayoutBuilder();

	~DescriptorSetLayoutBuilder()
	{
#ifdef _DEBUG
		std::cout << "DescriptorSetLayoutBuilder :: �f�X�g���N�^" << std::endl;
#endif
	}

	//�v���p�e�B�̏�����
	std::shared_ptr<DescriptorSetLayoutBuilder> initProperty();

	//�^�C�v�Ǝg���V�F�[�_�̐ݒ�
	std::shared_ptr<DescriptorSetLayoutBuilder> setProperty(VkDescriptorType type, VkShaderStageFlagBits stage);
	std::shared_ptr<DescriptorSetLayoutBuilder> setProperty(int type, int stage);

	//�o�C���f�B���O�̍쐬
	std::vector<VkDescriptorSetLayoutBinding> Build();
};