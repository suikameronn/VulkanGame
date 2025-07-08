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
	
	//�R�}���h�o�b�t�@�쐬�p
	std::shared_ptr<VulkanCore> vulkanCore;

	//�r���_�[
	std::unique_ptr<GpuBufferBuilder> builder;

	//�����f�o�C�X
	VkPhysicalDevice physicalDevice;

	//�_���f�o�C�X
	VkDevice device;

	//���݂̔j���\��̃��X�g�̃C���f�b�N�X
	//�`�揈�����s���邲�Ƃɒl���X�V�����
	uint32_t frameIndex;

	//�j���\��̃o�b�t�@���l�ߍ��܂��
	std::array<std::list<std::pair<VkBuffer, VkDeviceMemory>>, 2> destructList;

	//�����Ƃ��Ď󂯂�enum class����VkBufferUsageFlagBits�ɕϊ�����
	VkBufferUsageFlagBits convertUsageFlagBits(BufferUsage usage, BufferTransferType transferType);

	//�����Ƃ��Ď󂯂�enum class����VkMemoryPropertyFlagBits�ɕϊ�����
	VkMemoryPropertyFlagBits convertMemoryPropertyFlagBits(BufferUsage usage);

	//�������̃f�[�^���R�s�[
	void copyMemory(VkDeviceSize size, void* src, std::shared_ptr<GpuBuffer> dst, bool unmapped);
	void copyMemory(VkDeviceSize size, void* src, GpuBuffer& dst, bool unmapped);

	//�o�b�t�@�̓��e���R�s�[
	void copyBuffer(VkDeviceSize size, GpuBuffer& src, std::shared_ptr<GpuBuffer> dst);

public:

	GpuBufferFactory(std::shared_ptr<VulkanCore> vulkanCore);

	//�o�b�t�@�̐ݒ�𒼐ڎw�肵�āA�o�b�t�@���쐬����
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, void* srcPtr
		, VkBufferUsageFlags usage, VkMemoryPropertyFlags property);

	//���炩���ߐݒ肳�ꂽ�o�b�t�@�̐ݒ�Ńo�b�t�@���쐬����
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, void* srcPtr
		, BufferUsage usage,BufferTransferType transferType);

	//�x���j�����X�g�Ƀo�b�t�@��ǉ�����
	void addDefferedDestruct(VkBuffer& buffer, VkDeviceMemory& memory);

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