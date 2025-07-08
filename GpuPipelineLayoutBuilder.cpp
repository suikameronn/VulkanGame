#include"GpuPipelineLayoutBuilder.h"

GpuPipelineLayoutBuilder::GpuPipelineLayoutBuilder(VkDevice& d, std::shared_ptr<GpuDescriptorSetLayoutFactory> f)
{
	device = d;

    layoutFactory = f;
}

//レイアウトを初期化する
void GpuPipelineLayoutBuilder::initProperty()
{
    layoutArray.clear();
}

//DescriptorSetLayoutからレイアウトを積み上げる
void GpuPipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	layoutArray.push_back(layout);
}

//パイプラインレイアウトを作成する
void GpuPipelineLayoutBuilder::Create(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts)
{
	layouts = layoutArray;

	initProperty();
}