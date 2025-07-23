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
struct CommandBuffer;

class GpuBufferFactory:public std::enable_shared_from_this<GpuBufferFactory>
{
private:
	
	//�R�}���h�o�b�t�@�쐬�p
	std::shared_ptr<VulkanCore> vulkanCore;

	//�r���_�[
	std::shared_ptr<GpuBufferBuilder> builder;

	//�����f�o�C�X
	VkPhysicalDevice physicalDevice;

	//�_���f�o�C�X
	VkDevice device;

	//�R�}���h�v�[��
	VkCommandPool commandPool;

	//���݂̔j���\��̃��X�g�̃C���f�b�N�X
	//�`�揈�����s���邲�Ƃɒl���X�V�����
	uint32_t frameIndex;

	//�j���\��̃o�b�t�@���l�ߍ��܂��
	std::array<std::list<std::pair<VkBuffer, VkDeviceMemory>>, 2> destructList;
	std::array<std::list<VkCommandBuffer>, 2> destructListCommand;

	//�����Ƃ��Ď󂯂�enum class����VkBufferUsageFlagBits�ɕϊ�����
	VkBufferUsageFlagBits convertUsageFlagBits(BufferUsage usage, BufferTransferType transferType);

	//�����Ƃ��Ď󂯂�enum class����VkMemoryPropertyFlagBits�ɕϊ�����
	VkMemoryPropertyFlagBits convertMemoryPropertyFlagBits(BufferUsage usage);

public:

	GpuBufferFactory(std::shared_ptr<VulkanCore> vulkanCore, std::shared_ptr<GpuBufferBuilder> b);

	//�o�b�t�@�̐ݒ�𒼐ڎw�肵�āA�o�b�t�@���쐬����
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, const void* srcPtr
		, VkBufferUsageFlags usage, VkMemoryPropertyFlags property);

	//���炩���ߐݒ肳�ꂽ�o�b�t�@�̐ݒ�Ńo�b�t�@���쐬����
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, const void* srcPtr
		, BufferUsage usage,BufferTransferType transferType);

	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, BufferUsage usage
		, BufferTransferType transferType);

	//�R�}���h�o�b�t�@�[���쐬����
	std::shared_ptr<CommandBuffer> CommandBufferCreate();

	//�������̃f�[�^���R�s�[
	void copyMemory(const VkDeviceSize size, const void* src, std::shared_ptr<GpuBuffer> dst, const bool unmapped);
	void copyMemory(const VkDeviceSize size, const void* src, GpuBuffer& dst, const bool unmapped);

	//�o�b�t�@�̓��e���R�s�[
	void copyBuffer(const VkDeviceSize size, const GpuBuffer& src, std::shared_ptr<GpuBuffer> dst);

	//�x���j�����X�g�Ƀo�b�t�@��ǉ�����
	void addDefferedDestruct(VkBuffer& buffer, VkDeviceMemory& memory);
	void addDefferedDestruct(VkCommandBuffer& commandBuffer);

	//�o�b�t�@��j������
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