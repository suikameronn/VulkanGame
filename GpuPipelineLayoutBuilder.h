#pragma once

#include"GpuDescriptorSetLayoutFactory.h"

class GpuPipelineLayoutBuilder
{
private:

	//�_���f�o�C�X
	VkDevice device;

	//�p�C�v���C���Ɏg�����C�A�E�g�̃v���p�e�B
	std::vector<std::shared_ptr<DescriptorSetLayout>> layoutArray;

	//VkDescriptorSetLayout�̃t�@�N�g��
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;

public:

	GpuPipelineLayoutBuilder(VkDevice& d,std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutF);

	//���C�A�E�g������������
	void initProperty();

	//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
	void addLayout(const std::shared_ptr<DescriptorSetLayout> layout);

	//�p�C�v���C�����C�A�E�g���쐬����
	void Create(std::vector< std::shared_ptr<DescriptorSetLayout>>& layouts);
};