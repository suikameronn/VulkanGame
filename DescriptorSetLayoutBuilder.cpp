#include"DescriptorSetLayoutBuilder.h"

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(VkDevice& d)
{
	device = d;
}

//プロパティの初期化
DescriptorSetLayoutBuilder DescriptorSetLayoutBuilder::initProperty()
{
	properties.clear();

	return *this;
}

//タイプと使うシェーダの設定
DescriptorSetLayoutBuilder DescriptorSetLayoutBuilder::setProperty(VkDescriptorType type, VkShaderStageFlagBits stage)
{
	properties.push_back({ type,stage });

	return *this;
}

//フラグを複合させたとき用
DescriptorSetLayoutBuilder DescriptorSetLayoutBuilder::setProperty(int type, int stage)
{
	properties.push_back({ static_cast<VkDescriptorType>(type),static_cast<VkShaderStageFlagBits>(stage) });

	return *this;
}

//VkDescriptorSetLayoutの作成
std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBuilder::Build()
{
	if (properties.size() == 0)
	{
		throw std::runtime_error("DescriptorSetLayoutBuilder : Build() : プロパティが設定されていません");
	}

	std::vector<VkDescriptorSetLayoutBinding> bindings(properties.size());
	for (int i = 0; i < bindings.size(); i++)
	{
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = i;
		binding.descriptorCount = 1;
		binding.descriptorType = properties[i].first;
		binding.pImmutableSamplers = nullptr;
		binding.stageFlags = properties[i].second;

		bindings[i] = binding;
	}

	//プロパティの初期化
	initProperty();

	return bindings;
}