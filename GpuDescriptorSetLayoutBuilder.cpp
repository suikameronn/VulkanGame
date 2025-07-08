#include"GpuDescriptorSetLayoutBuilder.h"

GpuDescriptorSetLayoutBuilder::GpuDescriptorSetLayoutBuilder(VkDevice& d)
{
	device = d;
}

//�v���p�e�B�̏�����
void GpuDescriptorSetLayoutBuilder::initProperty()
{
	properties.clear();
}

//�^�C�v�Ǝg���V�F�[�_�̐ݒ�
void GpuDescriptorSetLayoutBuilder::setProperty(VkDescriptorType type, VkShaderStageFlagBits stage)
{
	properties.push_back({ type,stage });
}

//�t���O�𕡍��������Ƃ��p
void GpuDescriptorSetLayoutBuilder::setProperty(int type, int stage)
{
	properties.push_back({ static_cast<VkDescriptorType>(type),static_cast<VkShaderStageFlagBits>(stage) });
}

//VkDescriptorSetLayout�̍쐬
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

	//�v���p�e�B�̏�����
	initProperty();
}