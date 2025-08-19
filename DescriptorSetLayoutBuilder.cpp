#include"DescriptorSetLayoutBuilder.h"

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder()
{
}

//プロパティの初期化
std::shared_ptr<DescriptorSetLayoutBuilder> DescriptorSetLayoutBuilder::initProperty()
{
	properties.clear();

	return shared_from_this();
}

//タイプと使うシェーダの設定
std::shared_ptr<DescriptorSetLayoutBuilder> DescriptorSetLayoutBuilder::setProperty(VkDescriptorType type, VkShaderStageFlagBits stage)
{
	properties.push_back({ type,stage });

	return shared_from_this();
}

//フラグを複合させたとき用
std::shared_ptr<DescriptorSetLayoutBuilder> DescriptorSetLayoutBuilder::setProperty(int type, int stage)
{
	properties.push_back({ static_cast<VkDescriptorType>(type),static_cast<VkShaderStageFlagBits>(stage) });

	return shared_from_this();
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