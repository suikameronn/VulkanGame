#pragma once

#include"VulkanCore.h"


class GpuBufferBuilder
{
private:
	//�����f�o�C�X
	VkPhysicalDevice physicalDevice;

	//�_���f�o�C�X
	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	//gpu��̃���������ݒ�ɓK�����郁������T��
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propery);

public:

	GpuBufferBuilder(VkPhysicalDevice& p, VkDevice& d);

	void Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
		, VkBuffer& buffer, VkDeviceMemory& memory);
};