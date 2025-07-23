#pragma once

#include"DescriptorSetLayoutFactory.h"

class PipelineLayoutBuilder
{
private:

	//�p�C�v���C���Ɏg�����C�A�E�g�̃v���p�e�B
	std::vector<std::shared_ptr<DescriptorSetLayout>> layoutArray;

	//VkDescriptorSetLayout�̃t�@�N�g��
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;

public:

	PipelineLayoutBuilder();

	//���C�A�E�g������������
	PipelineLayoutBuilder initProperty();

	//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
	PipelineLayoutBuilder addLayout(const std::shared_ptr<DescriptorSetLayout> layout);

	//�p�C�v���C�����C�A�E�g���쐬����
	std::vector<std::shared_ptr<DescriptorSetLayout>> Build();
};