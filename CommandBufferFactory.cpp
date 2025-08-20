#include"CommandBufferFactory.h"

ComamndBufferFactory::ComamndBufferFactory(std::shared_ptr<VulkanCore> core)
{
	vulkanCore = core;

	device = vulkanCore->getLogicDevice();

	commandPool = vulkanCore->getCommandPool();
}

//VkCommandBuffer���쐬����
std::shared_ptr<CommandBuffer> ComamndBufferFactory::Create(const uint32_t& bufferCount)
{
	std::shared_ptr<CommandBuffer> commandBuffer = std::make_shared<CommandBuffer>(device, shared_from_this(), bufferCount);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer->getCommandCount());

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer->getCommand()) != VK_SUCCESS)
	{
		throw std::runtime_error("�R�}���h�o�b�t�@�̍쐬�Ɏ��s");
	}

	return commandBuffer;
}

//VkCommandBuffer���쐬����
std::shared_ptr<CommandBuffer> ComamndBufferFactory::Create()
{
	std::shared_ptr<CommandBuffer> commandBuffer = std::make_shared<CommandBuffer>(device, shared_from_this());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer->getCommandCount());

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer->getCommand()) != VK_SUCCESS)
	{
		throw std::runtime_error("�R�}���h�o�b�t�@�̍쐬�Ɏ��s");
	}

	return commandBuffer;
}

//�R�}���h�o�b�t�@�̋L�^���J�n����
void ComamndBufferFactory::Begin(VkCommandBuffer& commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
}

//�R�}���h�o�b�t�@�̋L�^���I������
void ComamndBufferFactory::End(VkCommandBuffer& commandBuffer)
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

//�R�}���h�o�b�t�@��Submit����
void ComamndBufferFactory::Submit(std::shared_ptr<CommandBuffer> commandBuffer,VkQueue& queue)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer->getCommandCount());
	submitInfo.pCommandBuffers = &commandBuffer->getCommand();

	//���̃R�}���h�o�b�t�@���ҋ@���ׂ��A�Z�}�t�H��ݒ�
	if (commandBuffer->getWaitSemaphores().size() > 0)
	{
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(commandBuffer->getWaitSemaphores().size());
		submitInfo.pWaitSemaphores = commandBuffer->getWaitSemaphores().data();
		submitInfo.pWaitDstStageMask = &commandBuffer->getWaitStageMask();
	}

	//�Z�}�t�H���쐬����Ă���ꍇ�A����Submit�ɃZ�}�t�H������t����
	if(commandBuffer->getSemaphore() != VK_NULL_HANDLE)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &commandBuffer->getSemaphore();
	}

	//�t�F���X�̏����ݒ�́AVK_NULL_HANDLE
	if (vkQueueSubmit(queue, 1, &submitInfo, commandBuffer->getFence()) != VK_SUCCESS)
	{
		throw std::runtime_error("Submit�Ɏ��s");
	}

	//�t�F���X���L���ȏꍇ�́A�ҋ@����
	if(commandBuffer->getFence() != VK_NULL_HANDLE)
	{
		//�t�F���X���ݒ肳��Ă���ꍇ�A�ҋ@����
		vkWaitForFences(device, 1, &commandBuffer->getFence(), VK_TRUE, UINT64_MAX);
	}
}

//�R�}���h�o�b�t�@��j������
void ComamndBufferFactory::Destruct(std::vector<VkCommandBuffer>& commandBuffer, VkSemaphore semaphore, VkFence fence)
{
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffer.size()), commandBuffer.data());

	vkDestroySemaphore(device, semaphore, nullptr);

	vkDestroyFence(device, fence, nullptr);
}

//�R�}���h�o�b�t�@�ɐςݏグ��ꂽ�o�b�t�@�����Z�b�g����
void ComamndBufferFactory::ResetCommand(std::shared_ptr<CommandBuffer> commandBuffer)
{
	commandBuffer->resetWaitCommand();

	for (uint32_t i = 0; i < commandBuffer->getCommandCount(); i++)
	{
		if (vkResetCommandBuffer(commandBuffer->getCommand(i), 0) != VK_SUCCESS)
		{
			throw std::runtime_error("�R�}���h�o�b�t�@�̃��Z�b�g�Ɏ��s");
		}
	}
}