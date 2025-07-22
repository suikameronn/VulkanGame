#include"PipelineLayoutBuilder.h"

PipelineLayoutBuilder::PipelineLayoutBuilder(VkDevice& d, std::shared_ptr<DescriptorSetLayoutFactory> f)
{
	device = d;

    layoutFactory = f;
}

//���C�A�E�g������������
PipelineLayoutBuilder PipelineLayoutBuilder::initProperty()
{
    layoutArray.clear();

	return *this;
}

//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
PipelineLayoutBuilder PipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	layoutArray.push_back(layout);

	return *this;
}

//�p�C�v���C�����C�A�E�g���쐬����
std::vector<std::shared_ptr<DescriptorSetLayout>> PipelineLayoutBuilder::Build()
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts = layoutArray;

	initProperty();

	return layouts;
}