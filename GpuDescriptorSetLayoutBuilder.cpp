#include"GpuDescriptorSetLayoutBuilder.h"

GpuDescriptorSetLayoutBuilder::GpuDescriptorSetLayoutBuilder(VkDevice& d)
{
	device = d;
}

//�v���p�e�B�̏�����
GpuDescriptorSetLayoutBuilder GpuDescriptorSetLayoutBuilder::initProperty()
{
	properties.clear();

	return *this;
}

//�^�C�v�Ǝg���V�F�[�_�̐ݒ�
GpuDescriptorSetLayoutBuilder GpuDescriptorSetLayoutBuilder::setProperty(VkDescriptorType type, VkShaderStageFlagBits stage)
{
	properties.push_back({ type,stage });

	return *this;
}

//�t���O�𕡍��������Ƃ��p
GpuDescriptorSetLayoutBuilder GpuDescriptorSetLayoutBuilder::setProperty(int type, int stage)
{
	properties.push_back({ static_cast<VkDescriptorType>(type),static_cast<VkShaderStageFlagBits>(stage) });

	return *this;
}

//VkDescriptorSetLayout�̍쐬
std::vector<VkDescriptorSetLayoutBinding> GpuDescriptorSetLayoutBuilder::Build()
{
	if (properties.size() == 0)
	{
		throw std::runtime_error("GpuDescriptorSetLayoutBuilder : Build() : �v���p�e�B���ݒ肳��Ă��܂���");
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

	//�v���p�e�B�̏�����
	initProperty();

	return bindings;
}