#pragma once

#include"GpuDescriptorSetLayoutFactory.h"

class GpuPipelineLayoutBuilder
{
private:

	//論理デバイス
	VkDevice device;

	//パイプラインに使うレイアウトのプロパティ
	std::vector<std::shared_ptr<DescriptorSetLayout>> layoutArray;

	//VkDescriptorSetLayoutのファクトリ
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;

public:

	GpuPipelineLayoutBuilder(VkDevice& d,std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutF);

	//レイアウトを初期化する
	GpuPipelineLayoutBuilder initProperty();

	//DescriptorSetLayoutからレイアウトを積み上げる
	GpuPipelineLayoutBuilder addLayout(const std::shared_ptr<DescriptorSetLayout> layout);

	//パイプラインレイアウトを作成する
	std::vector< std::shared_ptr<DescriptorSetLayout>> Build();
};