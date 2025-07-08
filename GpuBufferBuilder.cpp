#include"GpuBufferBuilder.h"

//gpu��̃���������ݒ�ɓK�����郁������T��
uint32_t GpuBufferBuilder::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	//�f�o�C�X��̎g�p�\�ȃ��������ׂĂ���A�ݒ�̓K�����郁������T��
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties)
			== properties)
		{
			return i;
		}
	}

	throw std::runtime_error("�K�����郁������������܂���ł���");
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
		throw std::runtime_error("�o�b�t�@�I�u�W�F�N�g�̍쐬�Ɏ��s���܂���");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("�o�b�t�@�̍쐬�Ɏ��s���܂���");
	}

	vkBindBufferMemory(device, buffer, memory, 0);
}