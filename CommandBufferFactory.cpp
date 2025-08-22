#include"CommandBufferFactory.h"

CommandBufferFactory::CommandBufferFactory(std::shared_ptr<VulkanCore> core)
{
	vulkanCore = core;

	device = vulkanCore->getLogicDevice();

	commandPool = vulkanCore->getCommandPool();
}

//���g��VkCommandBuffer���쐬����
std::vector<VkCommandBuffer> CommandBufferFactory::createCommandBuffer(const uint32_t& bufferCount)
{
	std::vector<VkCommandBuffer> commandBuffer(bufferCount);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffer.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("�R�}���h�o�b�t�@�̍쐬�Ɏ��s");
	}

	return commandBuffer;
}

//���g�̃t�F���X���쐬����
VkFence CommandBufferFactory::createFence()
{
	VkFence fence;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
	{
		throw std::runtime_error("�t�F���X�̍쐬�Ɏ��s");
	}

	return fence;
}

//���g�̃Z�}�t�H���쐬����
VkSemaphore CommandBufferFactory::createSemaphore()
{
	VkSemaphore semaphore;

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("vkCreateSemaphore : error");
	}
}

//�R�}���h�o�b�t�@��j������
void CommandBufferFactory::Destruct(std::vector<VkCommandBuffer>& commandBuffer, VkSemaphore semaphore, VkFence fence)
{
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffer.size()), commandBuffer.data());

	vkDestroySemaphore(device, semaphore, nullptr);

	vkDestroyFence(device, fence, nullptr);
}

void CommandBufferFactory::Destruct(VkSemaphore semaphore, VkFence fence)
{
	vkDestroySemaphore(device, semaphore, nullptr);

	vkDestroyFence(device, fence, nullptr);
}