#pragma once

#include"DescriptorSetLayoutFactory.h"

struct PipelineLayoutProperty
{
	//パイプラインに使うレイアウトのプロパティ
	std::vector<std::shared_ptr<DescriptorSetLayout>> layoutArray;

	//プッシュコンスタント
	std::vector<VkPushConstantRange> pushconstantArray;

	void initProperty()
	{
		layoutArray.clear();

		pushconstantArray.clear();
	}

	inline bool operator==(const PipelineLayoutProperty& other) const
	{
		if (layoutArray.size() != other.layoutArray.size())
		{
			return false;
		}

		for (int i = 0; i < layoutArray.size(); i++)
		{
			if (layoutArray[i]->hashKey != other.layoutArray[i]->hashKey
				 || layoutArray[i]->layout != other.layoutArray[i]->layout)
			{
				return false;
			}
		}

		if (pushconstantArray.size() != other.pushconstantArray.size())
		{
			return false;
		}

		for (int i = 0; i < pushconstantArray.size(); i++)
		{
			if (pushconstantArray[i].size != other.pushconstantArray[i].size
				|| pushconstantArray[i].stageFlags != other.pushconstantArray[i].stageFlags)
			{
				return false;
			}
		}

		return true;
	}
};

class PipelineLayoutBuilder
{
private:

	PipelineLayoutProperty property;

	//VkDescriptorSetLayoutのファクトリ
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;

public:

	PipelineLayoutBuilder();

	//レイアウトを初期化する
	PipelineLayoutBuilder initProperty();

	//DescriptorSetLayoutからレイアウトを積み上げる
	PipelineLayoutBuilder addLayout(const std::shared_ptr<DescriptorSetLayout> layout);

	//VkPushConstantを加える
	PipelineLayoutBuilder addPushConstant(const uint32_t& size, const VkShaderStageFlags& stage);

	//パイプラインレイアウトを作成する
	PipelineLayoutProperty Build();
};