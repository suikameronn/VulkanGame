#include"GpuPipelineFactory.h"

GpuPipelineFactory::GpuPipelineFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutFactory> f)
{
	device = d;

	pLayoutFactory = f;
}

//遅延破棄リストにリソースを追加する
void GpuPipelineFactory::addDefferedDestruct(VkPipeline& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//リソースを破棄する
void GpuPipelineFactory::resourceDestruct()
{
	for (auto& resource : destructList[frameIndex])
	{
		vkDestroyPipeline(device, resource, nullptr);
	}

	frameIndex = (frameIndex == 0) ? 1 : 0;
}