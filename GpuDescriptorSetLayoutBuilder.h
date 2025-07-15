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
	GpuDescriptorSetLayoutBuilder initProperty();

	//タイプと使うシェーダの設定
	GpuDescriptorSetLayoutBuilder setProperty(VkDescriptorType type, VkShaderStageFlagBits stage);
	GpuDescriptorSetLayoutBuilder setProperty(int type, int stage);

	//バインディングの作成
	std::vector<VkDescriptorSetLayoutBinding> Build();
};