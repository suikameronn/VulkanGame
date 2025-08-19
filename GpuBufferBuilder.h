#pragma once

#include"VulkanCore.h"


class GpuBufferBuilder
{
private:
	//物理デバイス
	VkPhysicalDevice physicalDevice;

	//論理デバイス
	VkDevice device;

	//gpu上のメモリから設定に適合するメモリを探す
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propery);

public:

	GpuBufferBuilder(VkPhysicalDevice& p, VkDevice& d);

	~GpuBufferBuilder()
	{
#ifdef _DEBUG
		std::cout << "GpuBufferBuilder :: デストラクタ" << std::endl;
#endif
	}

	void Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
		, VkBuffer& buffer, VkDeviceMemory& memory);
};