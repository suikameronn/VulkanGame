#include"GpuPipelineLayoutBuilder.h"

GpuPipelineLayoutBuilder::GpuPipelineLayoutBuilder(VkDevice& d, std::shared_ptr<GpuDescriptorSetLayoutFactory> f)
{
	device = d;

    layoutFactory = f;
}

//���C�A�E�g������������
GpuPipelineLayoutBuilder GpuPipelineLayoutBuilder::initProperty()
{
    layoutArray.clear();

	return *this;
}

//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
GpuPipelineLayoutBuilder GpuPipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	layoutArray.push_back(layout);

	return *this;
}

//�p�C�v���C�����C�A�E�g���쐬����
std::vector<std::shared_ptr<DescriptorSetLayout>> GpuPipelineLayoutBuilder::Build()
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts = layoutArray;

	initProperty();

	return layouts;
}