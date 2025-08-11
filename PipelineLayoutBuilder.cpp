#include"PipelineLayoutBuilder.h"

PipelineLayoutBuilder::PipelineLayoutBuilder()
{
}

//レイアウトを初期化する
PipelineLayoutBuilder PipelineLayoutBuilder::initProperty()
{
	property.initProperty();

	return *this;
}

//DescriptorSetLayoutからレイアウトを積み上げる
PipelineLayoutBuilder PipelineLayoutBuilder::addLayout(const std::shared_ptr<DescriptorSetLayout> layout)
{
	property.layoutArray.push_back(layout);

	return *this;
}

//VkPushConstantを加える
PipelineLayoutBuilder PipelineLayoutBuilder::addPushConstant(const uint32_t& size, const VkShaderStageFlags& stage)
{
	VkPushConstantRange info{};
	info.offset = 0;
	info.size = size;
	info.stageFlags = stage;

	property.pushconstantArray.push_back(info);

	return *this;
}

//パイプラインレイアウトを作成する
PipelineLayoutProperty PipelineLayoutBuilder::Build()
{
	return property;
}