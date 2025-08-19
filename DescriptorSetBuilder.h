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

class DescriptorSetBuilder : public std::enable_shared_from_this<DescriptorSetBuilder>
{
private:

	VkDevice device;

	VkDescriptorPool descriptorPool;

	DescriptorSetBufferProperty bufferProperty;
	DescriptorSetImageProperty imageProperty;

	DescriptorSetProperty property;

	//VkDescriptorSetPoolの作成
	void createDescriptorPool();

public:

	DescriptorSetBuilder(VkDevice& d);

	~DescriptorSetBuilder()
	{
#ifdef _DEBUG
		std::cout << "DescriptorSetBuilder :: デストラクタ" << std::endl;
#endif
	}

	DescriptorSetProperty Build();

	void Create(const DescriptorSetProperty& property, VkDescriptorSet& descriptorSet);

	//プロパティの初期化
	std::shared_ptr<DescriptorSetBuilder> initProperty();

	//VkDescriptorSetLayoutの設定
	std::shared_ptr<DescriptorSetBuilder> withDescriptorSetLayout(const std::shared_ptr<DescriptorSetLayout>& layout);

	//VkDescriptorSetの個数の設定
	std::shared_ptr<DescriptorSetBuilder> withDescriptorSetCount(const uint32_t& count);

	//VkDescriptorBufferInfoの設定///////

	std::shared_ptr<DescriptorSetBuilder> withBindingBuffer(const uint32_t& binding);

	std::shared_ptr<DescriptorSetBuilder> withTypeBuffer(const VkDescriptorType& type);

	std::shared_ptr<DescriptorSetBuilder> withBuffer(const std::shared_ptr<GpuBuffer>& buffer);

	std::shared_ptr<DescriptorSetBuilder> withOffset(const uint32_t& offset);

	std::shared_ptr<DescriptorSetBuilder> withRange(const uint32_t& range);

	std::shared_ptr<DescriptorSetBuilder> addBufferInfo();

	//VkDescriptorImageInfo/////////

	std::shared_ptr<DescriptorSetBuilder> withBindingImage(const uint32_t& binding);

	std::shared_ptr<DescriptorSetBuilder> withTypeImage(const VkDescriptorType& type);

	std::shared_ptr<DescriptorSetBuilder> withImageLayout(const VkImageLayout& layout);

	std::shared_ptr<DescriptorSetBuilder> withTexture(const std::shared_ptr<Texture> texture);
	std::shared_ptr<DescriptorSetBuilder> withTexture(const std::shared_ptr<Texture> texture, const uint32_t targetLayer);

	std::shared_ptr<DescriptorSetBuilder> addImageInfo();
};