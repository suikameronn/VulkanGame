#include"DescriptorSetBuilder.h"

DescriptorSetBuilder::DescriptorSetBuilder(std::shared_ptr<VulkanCore> core)
{
	vulkanCore = core;

	device = vulkanCore->getLogicDevice();

	createDescriptorPool();
}

//VkDescriptorSetPoolの作成
void DescriptorSetBuilder::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(100 * 2);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(100 * 2);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(1000);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

DescriptorSetProperty DescriptorSetBuilder::Build()
{
	DescriptorSetProperty p = property;

	initProperty();

	return p;
}

void DescriptorSetBuilder::Create(const DescriptorSetProperty& property, VkDescriptorSet& descriptorSet)
{
	if (vkAllocateDescriptorSets(device, &property.allocateInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("DescriptorSetBuilder : Create() : VkDescriptorSetの作成に失敗");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites(property.buffer.size() + property.texture.size());

	for (int i = 0; i < property.buffer.size(); ++i)
	{
		const auto& bufferProperty = property.buffer[i];

		descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i].dstSet = descriptorSet;
		descriptorWrites[i].dstBinding = bufferProperty.binding;
		descriptorWrites[i].dstArrayElement = 0;
		descriptorWrites[i].descriptorType = bufferProperty.type;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pBufferInfo = &bufferProperty.bufferInfo;
	}

	for (int i = 0; i < property.texture.size(); ++i)
	{
		const auto& imageProperty = property.texture[i];

		descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i].dstSet = descriptorSet;
		descriptorWrites[i].dstBinding = imageProperty.binding;
		descriptorWrites[i].dstArrayElement = 0;
		descriptorWrites[i].descriptorType = imageProperty.type;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pImageInfo = &imageProperty.imageInfo;
	}

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

//プロパティの初期化
DescriptorSetBuilder DescriptorSetBuilder::initProperty()
{
	bufferProperty.initProperty();
	imageProperty.initProperty();

	property.initProperty(descriptorPool);
}

//VkDescriptorSetLayoutの設定
DescriptorSetBuilder DescriptorSetBuilder::withDescriptorSetLayout(const std::shared_ptr<DescriptorSetLayout>& layout)
{
    property.layout = layout;
}

//VkDescriptorSetの個数の設定
DescriptorSetBuilder DescriptorSetBuilder::withDescriptorSetCount(const uint32_t& count)
{
	property.allocateInfo.descriptorSetCount = count;
}

//VkDescriptorBufferInfoの設定///////

DescriptorSetBuilder DescriptorSetBuilder::withBindingBuffer(const uint32_t& binding)
{
	bufferProperty.binding = binding;
}

DescriptorSetBuilder DescriptorSetBuilder::withTypeBuffer(const VkDescriptorType& type)
{
	bufferProperty.type = type;
}

DescriptorSetBuilder DescriptorSetBuilder::withBuffer(const std::shared_ptr<GpuBuffer>& buffer)
{
	bufferProperty.buffer = buffer;
}

DescriptorSetBuilder DescriptorSetBuilder::withOffset(const uint32_t& offset)
{
	bufferProperty.bufferInfo.offset = offset;
}

DescriptorSetBuilder DescriptorSetBuilder::withRange(const uint32_t& range)
{
	bufferProperty.bufferInfo.range = range;
}

DescriptorSetBuilder DescriptorSetBuilder::addBufferInfo()
{
	property.buffer.push_back(bufferProperty);

	bufferProperty.initProperty();
}

//VkDescriptorImageInfo/////////

DescriptorSetBuilder DescriptorSetBuilder::withBindingImage(const uint32_t& binding)
{
	imageProperty.binding = binding;
}

DescriptorSetBuilder DescriptorSetBuilder::withTypeImage(const VkDescriptorType& type)
{
	imageProperty.type = type;
}

DescriptorSetBuilder DescriptorSetBuilder::withImageLayout(const VkImageLayout& layout)
{
	imageProperty.imageInfo.imageLayout = layout;
}

DescriptorSetBuilder DescriptorSetBuilder::withTexture(const std::shared_ptr<Texture> texture)
{
	imageProperty.texture = texture;
}

DescriptorSetBuilder DescriptorSetBuilder::addImageInfo()
{
	property.texture.push_back(imageProperty);
	imageProperty.initProperty();
}