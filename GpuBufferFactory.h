#pragma once

#include"GpuBufferBuilder.h"

enum class BufferUsage
{
	VERTEX = 0,
	UIVERTEX,
	COLIDERVERTEX,
	INDEX,
	UNIFORM,
	LOCALSTORAGE,
	HOSTSTORAGE
};

enum class BufferTransferType
{
	SRC = 0,
	DST,
	SRCDST,
	NONE
};

struct GpuBuffer;

class GpuBufferFactory:public std::enable_shared_from_this<GpuBufferFactory>
{
private:
	
	//コマンドバッファ作成用
	std::shared_ptr<VulkanCore> vulkanCore;

	//ビルダー
	std::unique_ptr<GpuBufferBuilder> builder;

	//物理デバイス
	VkPhysicalDevice physicalDevice;

	//論理デバイス
	VkDevice device;

	//コマンドプール
	VkCommandPool commandPool;

	//現在の破棄予定のリストのインデックス
	//描画処理が行われるごとに値が更新される
	uint32_t frameIndex;

	//破棄予定のバッファが詰め込まれる
	std::array<std::list<std::pair<VkBuffer, VkDeviceMemory>>, 2> destructList;
	std::array<std::list<VkCommandBuffer>, 2> destructListCommand;

	//引数として受けたenum classからVkBufferUsageFlagBitsに変換する
	VkBufferUsageFlagBits convertUsageFlagBits(BufferUsage usage, BufferTransferType transferType);

	//引数として受けたenum classからVkMemoryPropertyFlagBitsに変換する
	VkMemoryPropertyFlagBits convertMemoryPropertyFlagBits(BufferUsage usage);

public:

	GpuBufferFactory(std::shared_ptr<VulkanCore> vulkanCore);

	//バッファの設定を直接指定して、バッファを作成する
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, const void* srcPtr
		, VkBufferUsageFlags usage, VkMemoryPropertyFlags property);

	//あらかじめ設定されたバッファの設定でバッファを作成する
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, const void* srcPtr
		, BufferUsage usage,BufferTransferType transferType);

	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, BufferUsage usage
		, BufferTransferType transferType);

	//コマンドバッファーを作成する
	std::shared_ptr<CommandBuffer> CommandBufferCreate();

	//メモリのデータをコピー
	void copyMemory(const VkDeviceSize size, const void* src, std::shared_ptr<GpuBuffer> dst, const bool unmapped);
	void copyMemory(const VkDeviceSize size, const void* src, GpuBuffer& dst, const bool unmapped);

	//バッファの内容をコピー
	void copyBuffer(const VkDeviceSize size, const GpuBuffer& src, std::shared_ptr<GpuBuffer> dst);

	//遅延破棄リストにバッファを追加する
	void addDefferedDestruct(VkBuffer& buffer, VkDeviceMemory& memory);
	void addDefferedDestruct(VkCommandBuffer& commandBuffer);

	//バッファを破棄する
	void resourceDestruct();
};

struct GpuBuffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* mappedPtr;
	std::shared_ptr<GpuBufferFactory> factory;

	GpuBuffer(std::shared_ptr<GpuBufferFactory> f)
	{
		factory = f;

		buffer = nullptr;
		memory = nullptr;
		mappedPtr = nullptr;
	}

	~GpuBuffer()
	{
		factory->addDefferedDestruct(buffer, memory);
	}
};

struct CommandBuffer
{


	VkCommandBuffer commandBuffer;

	std::shared_ptr<GpuBufferFactory> factory;

	CommandBuffer(std::shared_ptr<GpuBufferFactory> f)
	{
		factory = f;

		commandBuffer = nullptr;
	}

	~CommandBuffer()
	{
		factory->addDefferedDestruct(commandBuffer);
	}

	void reset()
	{
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
	}

	void finish()
	{
		vkEndCommandBuffer(commandBuffer);
	}
};