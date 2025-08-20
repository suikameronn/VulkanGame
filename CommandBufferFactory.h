#pragma once

#include"VulkanCore.h"

class CommandBuffer;

class ComamndBufferFactory : public std::enable_shared_from_this<ComamndBufferFactory>
{
private:

	std::shared_ptr<VulkanCore> vulkanCore;

	VkDevice device;

	VkCommandPool commandPool;

public:

	ComamndBufferFactory(std::shared_ptr<VulkanCore> core);

	//VkCommandBuffer���쐬����
	std::shared_ptr<CommandBuffer> Create(const uint32_t& bufferCount);
	std::shared_ptr<CommandBuffer> Create();

	//�R�}���h�o�b�t�@�̋L�^���J�n����
	void Begin(VkCommandBuffer& commandBuffer);

	//�R�}���h�o�b�t�@�̋L�^���I������
	void End(VkCommandBuffer& commandBuffer);

	//�R�}���h�o�b�t�@��Submit����
	void Submit(std::shared_ptr<CommandBuffer> commandBuffer, VkQueue& queue);

	//�R�}���h�o�b�t�@�𕡐�������Submit����
	void Submit(std::vector<std::shared_ptr<CommandBuffer>>& commandBuffers, VkQueue& queue);

	//�R�}���h�o�b�t�@��j������
	void Destruct(std::vector<VkCommandBuffer>& commandBuffer, VkSemaphore semaphore, VkFence fence);

	//�R�}���h�o�b�t�@�ɐςݏグ��ꂽ�o�b�t�@�����Z�b�g����
	void ResetCommand(std::shared_ptr<CommandBuffer> commandBuffer);
};

class CommandBuffer : std::enable_shared_from_this<CommandBuffer>
{
private:

	bool isSubmitted;

	std::vector<VkCommandBuffer> commandBuffer;

	VkSemaphore semaphore;

	VkPipelineStageFlags waitStageMask;

	VkFence fence;

	//�ҋ@����ׂ��A�R�}���h�o�b�t�@
	std::vector<std::shared_ptr<CommandBuffer>> waitPrevCommand;

	std::vector<VkSemaphore> waitSemaphores;

	//�X�R�[�v�O�֏o���ۂ̔j����h�~
	std::shared_ptr<CommandBuffer> thisPtr;

	VkDevice device;

	std::shared_ptr<ComamndBufferFactory> factory;

public:

	CommandBuffer(VkDevice& d,std::shared_ptr<ComamndBufferFactory> f)
	{
		device = d;

		isSubmitted = false;

		commandBuffer.resize(1, VK_NULL_HANDLE);

		semaphore = VK_NULL_HANDLE;

		fence = VK_NULL_HANDLE;

		waitPrevCommand.clear();

		std::fill(waitSemaphores.begin(), waitSemaphores.end(), VK_NULL_HANDLE);

		waitStageMask = 0;

		factory = f;

		thisPtr = shared_from_this();
	}

	CommandBuffer(VkDevice& d, std::shared_ptr<ComamndBufferFactory> f, const uint32_t bufferCount)
	{
		device = d;

		isSubmitted = false;

		commandBuffer.resize(bufferCount, VK_NULL_HANDLE);

		semaphore = VK_NULL_HANDLE;

		fence = VK_NULL_HANDLE;

		waitPrevCommand.clear();
		
		std::fill(waitSemaphores.begin(), waitSemaphores.end(), VK_NULL_HANDLE);

		waitStageMask = 0;

		factory = f;

		thisPtr = shared_from_this();
	}

	~CommandBuffer()
	{
		factory->Destruct(commandBuffer, semaphore, fence);
	}

	uint32_t getCommandCount() const
	{
		return static_cast<uint32_t>(commandBuffer.size());
	}

	VkCommandBuffer& getCommand()
	{
		return commandBuffer[0];
	}

	VkCommandBuffer& getCommand(const uint32_t& index)
	{
		return commandBuffer[index];
	}

	void setFence()
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS
			&& vkResetFences(device, 1, &fence) != VK_SUCCESS)
		{
			throw std::runtime_error("�t�F���X�̍쐬�Ɏ��s");
		}
	}

	void setFence(const VkFence& f)
	{
		fence = f;
	}

	void setSemaphore()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("vkCreateSemaphore : error");
		}
	}

	void setSemaphore(const VkSemaphore& s)
	{
		semaphore = s;
	}

	VkFence& getFence()
	{
		return fence;
	}

	VkSemaphore& getSemaphore()
	{
		return semaphore;
	}

	std::vector<VkSemaphore>& getWaitSemaphores()
	{
		return waitSemaphores;
	}

	const VkPipelineStageFlags& getWaitStageMask()
	{
		return waitStageMask;
	}

	void addWaitCommand(std::shared_ptr<CommandBuffer> command)
	{
		waitPrevCommand.push_back(command);

		if (command->getSemaphore() != VK_NULL_HANDLE)
		{
			waitSemaphores.push_back(command->getSemaphore());
		}

		if (command->getFence() != VK_NULL_HANDLE)
		{
			throw std::runtime_error("�L���ȃt�F���X�����ACommandBuffer�̒ǉ��͋������Ȃ�");
		}
	}

	void resetWaitCommand()
	{
		waitPrevCommand.clear();
	}

	void waitFence()
	{
		if(fence != VK_NULL_HANDLE && isSubmitted)
		{
			vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		}
	}
};