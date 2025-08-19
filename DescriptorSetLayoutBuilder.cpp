#include"DescriptorSetLayoutBuilder.h"

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder()
{
}

//�v���p�e�B�̏�����
std::shared_ptr<DescriptorSetLayoutBuilder> DescriptorSetLayoutBuilder::initProperty()
{
	properties.clear();

	return shared_from_this();
}

//�^�C�v�Ǝg���V�F�[�_�̐ݒ�
std::shared_ptr<DescriptorSetLayoutBuilder> DescriptorSetLayoutBuilder::setProperty(VkDescriptorType type, VkShaderStageFlagBits stage)
{
	properties.push_back({ type,stage });

	return shared_from_this();
}

//�t���O�𕡍��������Ƃ��p
std::shared_ptr<DescriptorSetLayoutBuilder> DescriptorSetLayoutBuilder::setProperty(int type, int stage)
{
	properties.push_back({ static_cast<VkDescriptorType>(type),static_cast<VkShaderStageFlagBits>(stage) });

	return shared_from_this();
}

//VkDescriptorSetLayout�̍쐬
std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBuilder::Build()
{
	if (properties.size() == 0)
	{
		throw std::runtime_error("DescriptorSetLayoutBuilder : Build() : �v���p�e�B���ݒ肳��Ă��܂���");
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