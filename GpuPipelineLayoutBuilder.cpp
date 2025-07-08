#include"GpuPipelineLayoutBuilder.h"

GpuPipelineLayoutBuilder::GpuPipelineLayoutBuilder(VkDevice& d, std::shared_ptr<GpuDescriptorSetLayoutFactory> f)
{
	device = d;

    layoutFactory = f;
}

//���C�A�E�g������������
void GpuPipelineLayoutBuilder::initProperty()
{
    layoutArray.clear();
}

//DescriptorSetLayout���烌�C�A�E�g��ςݏグ��
void GpuPipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	layoutArray.push_back(layout);
}

//�p�C�v���C�����C�A�E�g���쐬����
void GpuPipelineLayoutBuilder::Create(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts)
{
	layouts = layoutArray;

	initProperty();
}