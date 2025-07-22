#pragma once

#include"DescriptorSetLayoutFactory.h"

class PipelineLayoutBuilder
{
private:

	//論理デバイス
	VkDevice device;

	//パイプラインに使うレイアウトのプロパティ
	std::vector<std::shared_ptr<DescriptorSetLayout>> layoutArray;

	//VkDescriptorSetLayoutのファクトリ
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;

public:

	PipelineLayoutBuilder(VkDevice& d,std::shared_ptr<DescriptorSetLayoutFactory> layoutF);

	//レイアウトを初期化する
	PipelineLayoutBuilder initProperty();

	//DescriptorSetLayoutからレイアウトを積み上げる
	PipelineLayoutBuilder addLayout(const std::shared_ptr<DescriptorSetLayout> layout);

	//パイプラインレイアウトを作成する
	std::vector< std::shared_ptr<DescriptorSetLayout>> Build();
};