#include"CommandBufferFactory.h"

ComamndBufferFactory::ComamndBufferFactory(std::shared_ptr<VulkanCore> core)
{
	vulkanCore = core;

	device = vulkanCore->getLogicDevice();

	commandPool = vulkanCore->getCommandPool();
}

//VkCommandBufferを作成する
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
		throw std::runtime_error("コマンドバッファの作成に失敗");
	}

	return commandBuffer;
}

//VkCommandBufferを作成する
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
		throw std::runtime_error("コマンドバッファの作成に失敗");
	}

	return commandBuffer;
}

//コマンドバッファの記録を開始する
void ComamndBufferFactory::Begin(VkCommandBuffer& commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
}

//コマンドバッファの記録を終了する
void ComamndBufferFactory::End(VkCommandBuffer& commandBuffer)
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

//コマンドバッファをSubmitする
void ComamndBufferFactory::Submit(std::shared_ptr<CommandBuffer> commandBuffer,VkQueue& queue)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer->getCommandCount());
	submitInfo.pCommandBuffers = &commandBuffer->getCommand();

	//このコマンドバッファが待機すべき、セマフォを設定
	if (commandBuffer->getWaitSemaphores().size() > 0)
	{
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(commandBuffer->getWaitSemaphores().size());
		submitInfo.pWaitSemaphores = commandBuffer->getWaitSemaphores().data();
		submitInfo.pWaitDstStageMask = &commandBuffer->getWaitStageMask();
	}

	//セマフォが作成されている場合、このSubmitにセマフォを括り付ける
	if(commandBuffer->getSemaphore() != VK_NULL_HANDLE)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &commandBuffer->getSemaphore();
	}

	//フェンスの初期設定は、VK_NULL_HANDLE
	if (vkQueueSubmit(queue, 1, &submitInfo, commandBuffer->getFence()) != VK_SUCCESS)
	{
		throw std::runtime_error("Submitに失敗");
	}

	//フェンスが有効な場合は、待機する
	if(commandBuffer->getFence() != VK_NULL_HANDLE)
	{
		//フェンスが設定されている場合、待機する
		vkWaitForFences(device, 1, &commandBuffer->getFence(), VK_TRUE, UINT64_MAX);
	}
}

//コマンドバッファを破棄する
void ComamndBufferFactory::Destruct(std::vector<VkCommandBuffer>& commandBuffer, VkSemaphore semaphore, VkFence fence)
{
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffer.size()), commandBuffer.data());

	vkDestroySemaphore(device, semaphore, nullptr);

	vkDestroyFence(device, fence, nullptr);
}

//コマンドバッファに積み上げられたバッファをリセットする
void ComamndBufferFactory::ResetCommand(std::shared_ptr<CommandBuffer> commandBuffer)
{
	commandBuffer->resetWaitCommand();

	for (uint32_t i = 0; i < commandBuffer->getCommandCount(); i++)
	{
		if (vkResetCommandBuffer(commandBuffer->getCommand(i), 0) != VK_SUCCESS)
		{
			throw std::runtime_error("コマンドバッファのリセットに失敗");
		}
	}
}