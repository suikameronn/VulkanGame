#include"GpuDescriptorSetLayoutBuilder.h"

GpuDescriptorSetLayoutBuilder::GpuDescriptorSetLayoutBuilder(VkDevice& d)
{
	device = d;
}

//プロパティの初期化
void GpuDescriptorSetLayoutBuilder::initProperty()
{
	properties.clear();
}

//タイプと使うシェーダの設定
void GpuDescriptorSetLayoutBuilder::setProperty(VkDescriptorType type, VkShaderStageFlagBits stage)
{
	properties.push_back({ type,stage });
}

//フラグを複合させたとき用
void GpuDescriptorSetLayoutBuilder::setProperty(int type, int stage)
{
	properties.push_back({ static_cast<VkDescriptorType>(type),static_cast<VkShaderStageFlagBits>(stage) });
}

//VkDescriptorSetLayoutの作成
void GpuDescriptorSetLayoutBuilder::Create(std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	if (properties.size() == 0)
	{
		return;
	}

	bindings.resize(properties.size());
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
}