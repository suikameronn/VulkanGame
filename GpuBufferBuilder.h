#pragma once

#include"VulkanCore.h"


class GpuBufferBuilder
{
private:
	//�����f�o�C�X
	VkPhysicalDevice physicalDevice;

	//�_���f�o�C�X
	VkDevice device;

	//gpu��̃���������ݒ�ɓK�����郁������T��
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propery);

public:

	GpuBufferBuilder(VkPhysicalDevice& p, VkDevice& d);

	~GpuBufferBuilder()
	{
#ifdef _DEBUG
		std::cout << "GpuBufferBuilder :: �f�X�g���N�^" << std::endl;
#endif
	}

	void Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
		, VkBuffer& buffer, VkDeviceMemory& memory);
};