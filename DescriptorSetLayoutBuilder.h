#pragma once

#include<iostream>

#include"VulkanCore.h"

class DescriptorSetLayoutBuilder : public std::enable_shared_from_this<DescriptorSetLayoutBuilder>
{
private:

	//プロパティを保存する配列
	std::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>> properties;

public:

	DescriptorSetLayoutBuilder();

	~DescriptorSetLayoutBuilder()
	{
#ifdef _DEBUG
		std::cout << "DescriptorSetLayoutBuilder :: デストラクタ" << std::endl;
#endif
	}

	//プロパティの初期化
	std::shared_ptr<DescriptorSetLayoutBuilder> initProperty();

	//タイプと使うシェーダの設定
	std::shared_ptr<DescriptorSetLayoutBuilder> setProperty(VkDescriptorType type, VkShaderStageFlagBits stage);
	std::shared_ptr<DescriptorSetLayoutBuilder> setProperty(int type, int stage);

	//バインディングの作成
	std::vector<VkDescriptorSetLayoutBinding> Build();
};