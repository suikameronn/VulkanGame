#include"GpuPipelineFactory.h"

GpuPipelineFactory::GpuPipelineFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutFactory> f)
{
	device = d;

	pLayoutFactory = f;
}

//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
void GpuPipelineFactory::addDefferedDestruct(VkPipeline& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//���\�[�X��j������
void GpuPipelineFactory::resourceDestruct()
{
	for (auto& resource : destructList[frameIndex])
	{
		vkDestroyPipeline(device, resource, nullptr);
	}

	frameIndex = (frameIndex == 0) ? 1 : 0;
}