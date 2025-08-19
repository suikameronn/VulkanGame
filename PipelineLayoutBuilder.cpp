#include"PipelineLayoutBuilder.h"

PipelineLayoutBuilder::PipelineLayoutBuilder()
{
}

//���C�A�E�g������������
std::shared_ptr<PipelineLayoutBuilder> PipelineLayoutBuilder::initProperty()
{
	property.initProperty();

	return shared_from_this();
}

//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
std::shared_ptr<PipelineLayoutBuilder> PipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	property.layoutArray.push_back(layout);

	return shared_from_this();
}

//VkPushConstant��������
std::shared_ptr<PipelineLayoutBuilder> PipelineLayoutBuilder::addPushConstant(const uint32_t& size, const VkShaderStageFlags& stage)
{
	VkPushConstantRange info{};
	info.offset = 0;
	info.size = size;
	info.stageFlags = stage;

	property.pushconstantArray.push_back(info);

	return shared_from_this();
}

//�p�C�v���C�����C�A�E�g���쐬����
PipelineLayoutProperty PipelineLayoutBuilder::Build()
{
	return property;
}