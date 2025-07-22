#pragma once

#include"VulkanCore.h"
#include"DescriptorSetLayoutFactory.h"
#include"GpuBufferFactory.h"
#include"TextureFactory.h"

struct DescriptorSetBufferProperty
{
	uint32_t binding;

	VkDescriptorType type;

	std::shared_ptr<GpuBuffer> buffer;

	VkDescriptorBufferInfo bufferInfo;

	void initProperty()
	{
		binding = 0;

		type = VkDescriptorType{};

		buffer = nullptr;
		
		bufferInfo = VkDescriptorBufferInfo{};
	}
};

struct DescriptorSetImageProperty
{
	uint32_t binding;

	VkDescriptorType type;

	std::shared_ptr<Texture> texture;

	VkDescriptorImageInfo imageInfo;

	void initProperty()
	{
		binding = 0;

		type = VkDescriptorType{};

		texture = nullptr;

		imageInfo = VkDescriptorImageInfo{};
	}
};

struct DescriptorSetProperty
{
	VkDescriptorSetAllocateInfo allocateInfo;

	uint32_t infoIndex;

	std::shared_ptr<DescriptorSetLayout> layout;

	std::vector<DescriptorSetBufferProperty> buffer;

	std::vector<DescriptorSetImageProperty> texture;

	void initProperty(VkDescriptorPool& pool)
	{
		allocateInfo = VkDescriptorSetAllocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = pool;

		buffer.clear();
		texture.clear();
	}
};

class DescriptorSetBuilder
{
private:

	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	VkDescriptorPool descriptorPool;

	DescriptorSetBufferProperty bufferProperty;
	DescriptorSetImageProperty imageProperty;

	DescriptorSetProperty property;

	//VkDescriptorSetPool�̍쐬
	void createDescriptorPool();

public:

	DescriptorSetBuilder(std::shared_ptr<VulkanCore> core);

	DescriptorSetProperty Build();

	void Create(const DescriptorSetProperty& property, VkDescriptorSet& descriptorSet);

	//�v���p�e�B�̏�����
	DescriptorSetBuilder initProperty();

	//VkDescriptorSetLayout�̐ݒ�
	DescriptorSetBuilder withDescriptorSetLayout(const std::shared_ptr<DescriptorSetLayout>& layout);

	//VkDescriptorSet�̌��̐ݒ�
	DescriptorSetBuilder withDescriptorSetCount(const uint32_t& count);

	//VkDescriptorBufferInfo�̐ݒ�///////

	DescriptorSetBuilder withBindingBuffer(const uint32_t& binding);

	DescriptorSetBuilder withTypeBuffer(const VkDescriptorType& type);

	DescriptorSetBuilder withBuffer(const std::shared_ptr<GpuBuffer>& buffer);

	DescriptorSetBuilder withOffset(const uint32_t& offset);

	DescriptorSetBuilder withRange(const uint32_t& range);

	DescriptorSetBuilder addBufferInfo();

	//VkDescriptorImageInfo/////////

	DescriptorSetBuilder withBindingImage(const uint32_t& binding);

	DescriptorSetBuilder withTypeImage(const VkDescriptorType& type);

	DescriptorSetBuilder withImageLayout(const VkImageLayout& layout);

	DescriptorSetBuilder withTexture(const std::shared_ptr<Texture> texture);

	DescriptorSetBuilder addImageInfo();
};