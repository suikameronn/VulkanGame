#include"GpuPipelineLayoutBuilder.h"

GpuPipelineLayoutBuilder::GpuPipelineLayoutBuilder(VkDevice& d, std::shared_ptr<GpuDescriptorSetLayoutFactory> f)
{
	device = d;

    layoutFactory = f;
}

//レイアウトを初期化する
GpuPipelineLayoutBuilder GpuPipelineLayoutBuilder::initProperty()
{
    layoutArray.clear();

	return *this;
}

//DescriptorSetLayoutからレイアウトを積み上げる
GpuPipelineLayoutBuilder GpuPipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	layoutArray.push_back(layout);

	return *this;
}

//パイプラインレイアウトを作成する
std::vector<std::shared_ptr<DescriptorSetLayout>> GpuPipelineLayoutBuilder::Build()
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts = layoutArray;

	initProperty();

	return layouts;
}