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
	std::shared_ptr<GpuBufferBuilder> builder;

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

	//�X�e�[�W���O�o�b�t�@�̃������̃}�b�v�ƃA���}�b�v
	void memoryMap(GpuBuffer& bufffer, size_t bufferSize);
	void memoryUnMap(GpuBuffer& buffer);

public:

	GpuBufferFactory(std::shared_ptr<VulkanCore> vulkanCore, std::shared_ptr<GpuBufferBuilder> b);

	~GpuBufferFactory()
	{
		for (int i = 0; i < 2; i++)
		{
			resourceDestruct();
		}

#ifdef _DEBUG
		std::cout << "GpuBufferFactory :: �f�X�g���N�^" << std::endl;
#endif
	}

	//�o�b�t�@�̐ݒ�𒼐ڎw�肵�āA�o�b�t�@���쐬����
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, const void* srcPtr
		, VkBufferUsageFlags usage, VkMemoryPropertyFlags property);

	//���炩���ߐݒ肳�ꂽ�o�b�t�@�̐ݒ�Ńo�b�t�@���쐬����
	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, const void* srcPtr
		, BufferUsage usage,BufferTransferType transferType);

	std::shared_ptr<GpuBuffer> Create(VkDeviceSize bufferSize, BufferUsage usage
		, BufferTransferType transferType);

	//�X�e�[�W���O�o�b�t�@�̃������̃}�b�v�ƃA���}�b�v
	void memoryMap(std::shared_ptr<GpuBuffer> bufffer, size_t bufferSize);
	void memoryUnMap(std::shared_ptr<GpuBuffer> buffer);

	//�������̃f�[�^���R�s�[
	void copyMemory(const VkDeviceSize size, const void* src, std::shared_ptr<GpuBuffer> dst);
	void copyMemory(const VkDeviceSize size, const void* src, GpuBuffer& dst);

	//�o�b�t�@�̓��e���R�s�[
	void copyBuffer(const VkDeviceSize size, const GpuBuffer& src, std::shared_ptr<GpuBuffer> dst);

	//�x���j�����X�g�Ƀo�b�t�@��ǉ�����
	void addDefferedDestruct(VkBuffer buffer, VkDeviceMemory memory);

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