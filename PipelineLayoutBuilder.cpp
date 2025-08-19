#include"PipelineLayoutBuilder.h"

PipelineLayoutBuilder::PipelineLayoutBuilder()
{
}

//レイアウトを初期化する
std::shared_ptr<PipelineLayoutBuilder> PipelineLayoutBuilder::initProperty()
{
	property.initProperty();

	return shared_from_this();
}

//DescriptorSetLayoutからレイアウトを積み上げる
std::shared_ptr<PipelineLayoutBuilder> PipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	property.layoutArray.push_back(layout);

	return shared_from_this();
}

//VkPushConstantを加える
std::shared_ptr<PipelineLayoutBuilder> PipelineLayoutBuilder::addPushConstant(const uint32_t& size, const VkShaderStageFlags& stage)
{
	VkPushConstantRange info{};
	info.offset = 0;
	info.size = size;
	info.stageFlags = stage;

	property.pushconstantArray.push_back(info);

	return shared_from_this();
}

//パイプラインレイアウトを作成する
PipelineLayoutProperty PipelineLayoutBuilder::Build()
{
	return property;
}