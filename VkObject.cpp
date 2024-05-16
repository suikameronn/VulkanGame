#include"VkObject.h"

std::bitset<8> VkObject::getLayoutBit()
{
	return layoutBit;
}

void VkObject::setDescriptorSetLayout(VkDescriptorSetLayout* layout)
{
	this->layout = layout;
}

void VkObject::setDescriptorPool(VkDescriptorPool* pool)
{
	this->pool = pool;
}

void VkObject::setDescriptorSet(VkDescriptorSet* descriptorSet)
{
	this->descriptorSet = descriptorSet;
}

VkDescriptorSetLayout* VkObject::getDescriptorSetLayout()
{
	return this->layout;
}

VkBuffer* VkObject::getUniformBuffer()
{
	return &uniformBuffer;
}