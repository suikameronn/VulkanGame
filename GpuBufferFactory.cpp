#include"GpuBufferFactory.h"

GpuBufferFactory::GpuBufferFactory(std::shared_ptr<VulkanCore> core, std::shared_ptr<GpuBufferBuilder> b)
{
	vulkanCore = core;

	//各種デバイスを取得
	physicalDevice = core->getPhysicalDevice();
	device = core->getLogicDevice();

	commandPool = core->getCommandPool();

	//初回のフレームインデックスを1に設定する
	frameIndex = 1;

	//ビルダーを作成する
	builder = b;
}

//バッファの内容をコピー
void GpuBufferFactory::copyBuffer(const VkDeviceSize size, const GpuBuffer& src
	, std::shared_ptr<GpuBuffer> dst)
{
	VkCommandBuffer commandBuffer = vulkanCore->beginSingleTimeCommandBuffer(); //vulkanCoreからコマンドバッファを持ってくる

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src.buffer, dst->buffer, 1, &copyRegion);

	//コマンドバッファの後処理
	vulkanCore->endSingleTimeCommandBuffer(commandBuffer);
}

//メモリのデータをコピー
void GpuBufferFactory::copyMemory(const VkDeviceSize size, const void* src
	, std::shared_ptr<GpuBuffer> dst)
{
	memcpy(dst->mappedPtr, src, size);
}

//ステージングバッファをコピー先として想定
void GpuBufferFactory::copyMemory(const VkDeviceSize size, const void* src, GpuBuffer& dst)
{
	memcpy(dst.mappedPtr, src, size);
}

//引数として受けたenum classからVkBufferUsageFlagBitsに変換する
VkBufferUsageFlagBits GpuBufferFactory::convertUsageFlagBits(BufferUsage usage,BufferTransferType transferType)
{
	VkBufferUsageFlagBits usageFlag;

	if (usage == BufferUsage::VERTEX)
	{
		usageFlag = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	}
	else if (usage == BufferUsage::UIVERTEX || usage == BufferUsage::COLIDERVERTEX)
	{
		usageFlag = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}
	else if (usage == BufferUsage::INDEX)
	{
		usageFlag = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}
	else if (usage == BufferUsage::UNIFORM)
	{
		usageFlag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	else if (usage == BufferUsage::LOCALSTORAGE)
	{
		usageFlag = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	}
	else if (usage == BufferUsage::HOSTSTORAGE)
	{
		usageFlag = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	}

	VkBufferUsageFlagBits transferFlag;

	if (transferType == BufferTransferType::SRC)
	{
		transferFlag = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}
	else if (transferType == BufferTransferType::DST)
	{
		transferFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	else if (transferType == BufferTransferType::SRCDST)
	{
		transferFlag = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}
	else if (transferType == BufferTransferType::NONE)
	{
		transferFlag = static_cast<VkBufferUsageFlagBits>(0);
	}

	return static_cast<VkBufferUsageFlagBits>(usageFlag | transferFlag);
}

//引数として受けたenum classからVkBufferUsageFlagBitsに変換する
VkMemoryPropertyFlagBits GpuBufferFactory::convertMemoryPropertyFlagBits(BufferUsage usage)
{
	if (usage == BufferUsage::VERTEX || usage == BufferUsage::UIVERTEX
		|| usage == BufferUsage::INDEX || usage == BufferUsage::LOCALSTORAGE)
	{
		//ステージングバッファが必要なグループ

		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
	else if (usage == BufferUsage::UNIFORM || usage == BufferUsage::HOSTSTORAGE)
	{
		//ステージングバッファが不要なグループ

		return static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
}

//ステージングバッファのメモリのマップとアンマップ
void GpuBufferFactory::memoryMap(GpuBuffer& bufffer)
{
	vkMapMemory(device, bufffer.memory, 0, VK_WHOLE_SIZE, 0, &bufffer.mappedPtr);
}

void GpuBufferFactory::memoryUnMap(GpuBuffer& buffer)
{
	vkUnmapMemory(device, buffer.memory);
}

//public////////////////////////////////////

//ステージングバッファのメモリのマップとアンマップ
void GpuBufferFactory::memoryMap(std::shared_ptr<GpuBuffer> bufffer)
{
	vkMapMemory(device, bufffer->memory, 0, VK_WHOLE_SIZE, 0, &bufffer->mappedPtr);
}

void GpuBufferFactory::memoryUnMap(std::shared_ptr<GpuBuffer> buffer)
{
	vkUnmapMemory(device, buffer->memory);
}

//バッファの設定を直接指定して、バッファを作成する
std::shared_ptr<GpuBuffer> GpuBufferFactory::Create(VkDeviceSize bufferSize, const void* srcPtr
	, VkBufferUsageFlags usage, VkMemoryPropertyFlags property)
{
	//この構造体を返す
	std::shared_ptr<GpuBuffer> buffer = std::make_shared<GpuBuffer>(shared_from_this());

	//まず作成するバッファがgpuローカルのメモリーかどうかを調べる
	if (property & 1)
	{
		//ステージングバッファを用意する
		GpuBuffer stagingBuffer(shared_from_this());

		builder->Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			, stagingBuffer.buffer, stagingBuffer.memory);
		
		//ローカルのバッファを作成する
		builder->Create(bufferSize, usage, property, buffer->buffer, buffer->memory);

		//ステージングバッファにデータをコピーする
		memoryMap(buffer);
		copyMemory(bufferSize, srcPtr, stagingBuffer);
		memoryUnMap(buffer);
		
		//ステージングバッファのデータをローカルのバッファにコピーする
		copyBuffer(bufferSize, stagingBuffer, buffer);
	}
	else
	{
		//ステージングバッファは必要ない


		//バッファを用意する
		builder->Create(bufferSize, usage, property, buffer->buffer, buffer->memory);
		
		//メモリのデータをバッファにコピーする
		memoryMap(buffer);
		copyMemory(bufferSize, srcPtr, buffer);
	}

	return buffer;
}

//あらかじめ設定されたバッファの設定でバッファを作成する
std::shared_ptr<GpuBuffer> GpuBufferFactory::Create(VkDeviceSize bufferSize, const void* srcPtr
	, BufferUsage usage, BufferTransferType transferType)
{
	//この構造体を返す
	std::shared_ptr<GpuBuffer> buffer = std::make_shared<GpuBuffer>(shared_from_this());

	if (usage == BufferUsage::VERTEX || usage == BufferUsage::INDEX ||
		usage == BufferUsage::UIVERTEX || usage == BufferUsage::LOCALSTORAGE)
	{
		//ステージングバッファを用意する
		GpuBuffer staging(shared_from_this());

		builder->Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging.buffer, staging.memory);

		//ローカルのバッファを作成する
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);

		memoryMap(staging); //ステージングバッファのメモリをマップする
		//データをステージングバッファにデータをコピーする
		copyMemory(bufferSize, srcPtr, staging);
		memoryUnMap(staging); //ステージングバッファのメモリをアンマップする

		//ステージングバッファのデータをローカルのバッファにコピーする
		copyBuffer(bufferSize, staging, buffer);
	}
	else
	{
		//ステージングバッファは必要ない

		//バッファを用意する
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);

		//メモリのデータをバッファにコピーする
		//バッファーはマップしたままで、cpu側からアクセスできるようにする
		memoryMap(buffer);
		copyMemory(bufferSize, srcPtr, buffer);
	}

	return buffer;
}

std::shared_ptr<GpuBuffer> GpuBufferFactory::Create(VkDeviceSize bufferSize, BufferUsage usage
	, BufferTransferType transferType)
{
	//この構造体を返す
	std::shared_ptr<GpuBuffer> buffer = std::make_shared<GpuBuffer>(shared_from_this());

	if (usage == BufferUsage::VERTEX || usage == BufferUsage::INDEX ||
		usage == BufferUsage::UIVERTEX || usage == BufferUsage::LOCALSTORAGE)
	{
		//ステージングバッファを用意する
		GpuBuffer staging(shared_from_this());

		builder->Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging.buffer, staging.memory);

		//ローカルのバッファを作成する
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);
	}
	else
	{
		//ステージングバッファは必要ない

		//バッファを用意する
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);
	}

	return buffer;
}

//コマンドバッファーを作成する
std::shared_ptr<CommandBuffer> GpuBufferFactory::CommandBufferCreate()
{
	std::shared_ptr<CommandBuffer> commandBuffer = std::make_shared<CommandBuffer>(shared_from_this());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer->commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	return commandBuffer;
}

//遅延破棄リストにバッファを追加する
void GpuBufferFactory::addDefferedDestruct(VkBuffer buffer, VkDeviceMemory memory)
{
	destructList[frameIndex].push_back({ buffer,memory });
}

void GpuBufferFactory::addDefferedDestruct(VkCommandBuffer commandBuffer)
{
	destructListCommand[frameIndex].push_back(commandBuffer);
}

//バッファを破棄する
void GpuBufferFactory::resourceDestruct()
{
	//実際にバッファを破棄する
	for (auto& bufferMemory : destructList[frameIndex])
	{
		vkDestroyBuffer(device, bufferMemory.first, nullptr);
		vkFreeMemory(device, bufferMemory.second, nullptr);
	}

	for (auto& command : destructListCommand[frameIndex])
	{
		vkFreeCommandBuffers(device, commandPool, 1, &command);
	}

	destructList[frameIndex].clear();

	//フレームインデックスを更新する
	frameIndex = (frameIndex == 0) ? 1 : 0;
}