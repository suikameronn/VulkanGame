#pragma once

#include"VulkanCore.h"

#include<chrono>

class CommandBuffer;

class CommandBufferFactory : public std::enable_shared_from_this<CommandBufferFactory>
{
private:

	std::shared_ptr<VulkanCore> vulkanCore;

	VkDevice device;

	VkCommandPool commandPool;

public:

	CommandBufferFactory(std::shared_ptr<VulkanCore> core);

	std::vector<VkCommandBuffer> createCommandBuffer(const uint32_t& bufferCount);
	VkFence createFence();
	VkSemaphore createSemaphore();

	//コマンドバッファを破棄する
	void Destruct(std::vector<VkCommandBuffer>& commandBuffer, VkSemaphore semaphore, VkFence fence);
	void Destruct(VkSemaphore semaphore, VkFence fence);
};

class CommandBuffer : public std::enable_shared_from_this<CommandBuffer>
{
private:

	bool isSubmitted;

	std::vector<VkCommandBuffer> commandBuffer;

	VkSemaphore semaphore;

	VkPipelineStageFlags waitStageMask;

	VkFence fence;

	//待機するべき、コマンドバッファ
	std::vector<std::shared_ptr<CommandBuffer>> waitPrevCommand;

	std::pair<std::vector<VkSemaphore>, std::vector<VkPipelineStageFlags>> waitSemaphores;

	//スコープ外へ出た際の破棄を防止
	std::shared_ptr<CommandBuffer> thisPtr;

	VkDevice device;

	std::shared_ptr<CommandBufferFactory> factory;

	void resetWaitCommand()
	{
		isSubmitted = false;

		waitPrevCommand.clear();

		waitSemaphores.first.clear();
		waitSemaphores.second.clear();
	}

public:

	CommandBuffer(VkDevice& d, std::shared_ptr<CommandBufferFactory> f)
	{
		device = d;

		isSubmitted = false;

		commandBuffer.clear();

		semaphore = VK_NULL_HANDLE;

		fence = VK_NULL_HANDLE;

		waitPrevCommand.clear();

		waitStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		factory = f;
	}

	~CommandBuffer()
	{
		factory->Destruct(commandBuffer, semaphore, fence);
	}

	void releaseMyPtr()
	{
		//自身への参照を破棄する
		thisPtr.reset();
	}

	std::shared_ptr<CommandBuffer> setCommandBufffer(std::vector<VkCommandBuffer> src)
	{
		commandBuffer.resize(src.size());
		std::copy(src.begin(), src.end(), commandBuffer.begin());

		return shared_from_this();
	}

	std::shared_ptr<CommandBuffer> setFence(const VkFence& f)
	{
		fence = f;

		return shared_from_this();
	}

	std::shared_ptr<CommandBuffer> setSemaphore(const VkSemaphore& s, const VkPipelineStageFlags& flag)
	{
		semaphore = s;

		waitStageMask = flag;

		return shared_from_this();
	}

	std::shared_ptr<CommandBuffer> addWaitCommand(std::shared_ptr<CommandBuffer> command)
	{
		waitPrevCommand.push_back(command);

		if(command->getSemaphore() != VK_NULL_HANDLE)
		{
			waitSemaphores.first.push_back(command->getSemaphore());
			waitSemaphores.second.push_back(command->getWaitStageMask());
		}

		return shared_from_this();
	}

	std::shared_ptr<CommandBuffer> addWaitSemaphore(const VkSemaphore& semaphore, const VkPipelineStageFlags& waitStageMask)
	{
		waitSemaphores.first.push_back(semaphore);
		waitSemaphores.second.push_back(waitStageMask);

		return shared_from_this();
	}

	const uint32_t getCommandCount() const
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

	VkFence& getFence()
	{
		return fence;
	}

	VkSemaphore& getSemaphore()
	{
		return semaphore;
	}

	VkPipelineStageFlags& getWaitStageMask()
	{
		return waitStageMask;
	}

	std::pair<std::vector<VkSemaphore>, std::vector<VkPipelineStageFlags>>& getWaitSemaphores()
	{
		return waitSemaphores;
	}

	void waitFence()
	{
		if(fence != VK_NULL_HANDLE && isSubmitted)
		{
			auto start = std::chrono::high_resolution_clock::now();
			if (vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			{
				// 時間計測の終了
				auto end = std::chrono::high_resolution_clock::now();

				// 経過時間の計算（ミリ秒単位）
				std::chrono::duration<double, std::milli> elapsed = end - start;

				std::cout << "vkWaitForFences 待機時間: " << elapsed.count() << " ms" << std::endl;


				throw std::runtime_error("failed to submit draw command buffer!");
			}

			vkResetFences(device, 1, &fence);

			thisPtr.reset();

			resetWaitCommand();

			for(VkCommandBuffer& command : commandBuffer)
			{
				vkResetCommandBuffer(command, 0);
			}

			isSubmitted = false;
		}
	}

	void recordBegin()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer[0], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}

	void recordBegin(const uint32_t& index)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer[index], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}

	void recordEnd()
	{
		if (vkEndCommandBuffer(commandBuffer[0]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void recordEnd(const uint32_t& index)
	{
		if (vkEndCommandBuffer(commandBuffer[index]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void Submit(const VkQueue& queue)
	{
		if (isSubmitted)
		{
			return;
		}

		if (fence != VK_NULL_HANDLE)
		{
			vkResetFences(device, 1, &fence);
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());
		submitInfo.pCommandBuffers = commandBuffer.data();

		//このコマンドバッファが待機すべき、セマフォを設定
		if (waitSemaphores.first.size() > 0)
		{
			submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.first.size());
			submitInfo.pWaitSemaphores = waitSemaphores.first.data();
			submitInfo.pWaitDstStageMask = waitSemaphores.second.data();
		}

		//セマフォが作成されている場合、このSubmitにセマフォを括り付ける
		if (semaphore != VK_NULL_HANDLE)
		{
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &semaphore;
		}

		//フェンスの初期設定は、VK_NULL_HANDLE
		if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
		{
			throw std::runtime_error("Submitに失敗");
		}

		//最後尾のフェンスを持つ、コマンドバッファが破棄されたときに、
		//連鎖して破棄されるようにする
		if (fence == VK_NULL_HANDLE)
		{
			thisPtr.reset();
		}
		else
		{
			thisPtr = shared_from_this();
		}

		isSubmitted = true;
	}
};