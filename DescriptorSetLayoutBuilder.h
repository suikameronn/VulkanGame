#pragma once

#include<iostream>

#include"VulkanCore.h"

class DescriptorSetLayoutBuilder
{
private:

	//論理デバイス
	VkDevice device;

	//プロパティを保存する配列
	std::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>> properties;

public:

	DescriptorSetLayoutBuilder(VkDevice& d);

	//プロパティの初期化
	DescriptorSetLayoutBuilder initProperty();

	//タイプと使うシェーダの設定
	DescriptorSetLayoutBuilder setProperty(VkDescriptorType type, VkShaderStageFlagBits stage);
	DescriptorSetLayoutBuilder setProperty(int type, int stage);

	//バインディングの作成
	std::vector<VkDescriptorSetLayoutBinding> Build();
};