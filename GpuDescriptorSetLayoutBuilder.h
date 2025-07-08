#pragma once

#include<iostream>

#include"VulkanCore.h"

class GpuDescriptorSetLayoutBuilder
{
private:

	//論理デバイス
	VkDevice device;

	//プロパティを保存する配列
	std::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>> properties;

public:

	GpuDescriptorSetLayoutBuilder(VkDevice& d);

	//プロパティの初期化
	void initProperty();

	//タイプと使うシェーダの設定
	void setProperty(VkDescriptorType type, VkShaderStageFlagBits stage);
	void setProperty(int type, int stage);

	//バインディングの作成
	void Create(std::vector<VkDescriptorSetLayoutBinding>& bindings);
};