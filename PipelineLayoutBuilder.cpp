#include"PipelineLayoutBuilder.h"

PipelineLayoutBuilder::PipelineLayoutBuilder()
{
}

//���C�A�E�g������������
PipelineLayoutBuilder PipelineLayoutBuilder::initProperty()
{
	property.initProperty();

	return *this;
}

//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
PipelineLayoutBuilder PipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	property.layoutArray.push_back(layout);

	return *this;
}

//VkPushConstant��������
PipelineLayoutBuilder PipelineLayoutBuilder::addPushConstant(const uint32_t& size, const VkShaderStageFlags& stage)
{
	VkPushConstantRange info{};
	info.offset = 0;
	info.size = size;
	info.stageFlags = stage;

	property.pushconstantArray.push_back(info);

	return *this;
}

//�p�C�v���C�����C�A�E�g���쐬����
PipelineLayoutProperty PipelineLayoutBuilder::Build()
{
	return property;
}