#include"PipelineLayoutBuilder.h"

PipelineLayoutBuilder::PipelineLayoutBuilder(VkDevice& d, std::shared_ptr<DescriptorSetLayoutFactory> f)
{
	device = d;

    layoutFactory = f;
}

//レイアウトを初期化する
PipelineLayoutBuilder PipelineLayoutBuilder::initProperty()
{
    layoutArray.clear();

	return *this;
}

//DescriptorSetLayoutからレイアウトを積み上げる
PipelineLayoutBuilder PipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	layoutArray.push_back(layout);

	return *this;
}

//パイプラインレイアウトを作成する
std::vector<std::shared_ptr<DescriptorSetLayout>> PipelineLayoutBuilder::Build()
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts = layoutArray;

	initProperty();

	return layouts;
}