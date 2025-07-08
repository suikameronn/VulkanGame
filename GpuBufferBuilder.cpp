#include"GpuBufferBuilder.h"

//gpu上のメモリから設定に適合するメモリを探す
uint32_t GpuBufferBuilder::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	//デバイス上の使用可能なメモリすべてから、設定の適合するメモリを探す
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties)
			== properties)
		{
			return i;
		}
	}

	throw std::runtime_error("適合するメモリが見つかりませんでした");
}

//public///////////////////////////////////

GpuBufferBuilder::GpuBufferBuilder(VkPhysicalDevice& p, VkDevice& d)
{
	physicalDevice = p;
	device = d;
}

void GpuBufferBuilder::Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
	, VkBuffer& buffer, VkDeviceMemory& memory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("バッファオブジェクトの作成に失敗しました");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("バッファの作成に失敗しました");
	}

	vkBindBufferMemory(device, buffer, memory, 0);
}