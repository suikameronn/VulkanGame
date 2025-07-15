#include"GpuDescriptorSetLayoutBuilder.h"

GpuDescriptorSetLayoutBuilder::GpuDescriptorSetLayoutBuilder(VkDevice& d)
{
	device = d;
}

//プロパティの初期化
GpuDescriptorSetLayoutBuilder GpuDescriptorSetLayoutBuilder::initProperty()
{
	properties.clear();

	return *this;
}

//タイプと使うシェーダの設定
GpuDescriptorSetLayoutBuilder GpuDescriptorSetLayoutBuilder::setProperty(VkDescriptorType type, VkShaderStageFlagBits stage)
{
	properties.push_back({ type,stage });

	return *this;
}

//フラグを複合させたとき用
GpuDescriptorSetLayoutBuilder GpuDescriptorSetLayoutBuilder::setProperty(int type, int stage)
{
	properties.push_back({ static_cast<VkDescriptorType>(type),static_cast<VkShaderStageFlagBits>(stage) });

	return *this;
}

//VkDescriptorSetLayoutの作成
std::vector<VkDescriptorSetLayoutBinding> GpuDescriptorSetLayoutBuilder::Build()
{
	if (properties.size() == 0)
	{
		throw std::runtime_error("GpuDescriptorSetLayoutBuilder : Build() : プロパティが設定されていません");
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