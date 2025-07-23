#include"GpuBufferFactory.h"

GpuBufferFactory::GpuBufferFactory(std::shared_ptr<VulkanCore> core, std::shared_ptr<GpuBufferBuilder> b)
{
	vulkanCore = core;

	//�e��f�o�C�X���擾
	physicalDevice = core->getPhysicalDevice();
	device = core->getLogicDevice();

	commandPool = core->getCommandPool();

	//����̃t���[���C���f�b�N�X��1�ɐݒ肷��
	frameIndex = 1;

	//�r���_�[���쐬����
	builder = b;
}

//�o�b�t�@�̓��e���R�s�[
void GpuBufferFactory::copyBuffer(const VkDeviceSize size, const GpuBuffer& src
	, std::shared_ptr<GpuBuffer> dst)
{
	VkCommandBuffer commandBuffer = vulkanCore->beginSingleTimeCommandBuffer(); //vulkanCore����R�}���h�o�b�t�@�������Ă���

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src.buffer, dst->buffer, 1, &copyRegion);

	//�R�}���h�o�b�t�@�̌㏈��
	vulkanCore->endSingleTimeCommandBuffer(commandBuffer);
}

//�������̃f�[�^���R�s�[
void GpuBufferFactory::copyMemory(const VkDeviceSize size, const void* src
	, std::shared_ptr<GpuBuffer> dst)
{
	memcpy(dst->mappedPtr, src, size);
}

//�X�e�[�W���O�o�b�t�@���R�s�[��Ƃ��đz��
void GpuBufferFactory::copyMemory(const VkDeviceSize size, const void* src, GpuBuffer& dst)
{
	memcpy(dst.mappedPtr, src, size);
}

//�����Ƃ��Ď󂯂�enum class����VkBufferUsageFlagBits�ɕϊ�����
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

//�����Ƃ��Ď󂯂�enum class����VkBufferUsageFlagBits�ɕϊ�����
VkMemoryPropertyFlagBits GpuBufferFactory::convertMemoryPropertyFlagBits(BufferUsage usage)
{
	if (usage == BufferUsage::VERTEX || usage == BufferUsage::UIVERTEX
		|| usage == BufferUsage::INDEX || usage == BufferUsage::LOCALSTORAGE)
	{
		//�X�e�[�W���O�o�b�t�@���K�v�ȃO���[�v

		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
	else if (usage == BufferUsage::UNIFORM || usage == BufferUsage::HOSTSTORAGE)
	{
		//�X�e�[�W���O�o�b�t�@���s�v�ȃO���[�v

		return static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
}

//�X�e�[�W���O�o�b�t�@�̃������̃}�b�v�ƃA���}�b�v
void GpuBufferFactory::memoryMap(GpuBuffer& bufffer)
{
	vkMapMemory(device, bufffer.memory, 0, VK_WHOLE_SIZE, 0, &bufffer.mappedPtr);
}

void GpuBufferFactory::memoryUnMap(GpuBuffer& buffer)
{
	vkUnmapMemory(device, buffer.memory);
}

//public////////////////////////////////////

//�X�e�[�W���O�o�b�t�@�̃������̃}�b�v�ƃA���}�b�v
void GpuBufferFactory::memoryMap(std::shared_ptr<GpuBuffer> bufffer)
{
	vkMapMemory(device, bufffer->memory, 0, VK_WHOLE_SIZE, 0, &bufffer->mappedPtr);
}

void GpuBufferFactory::memoryUnMap(std::shared_ptr<GpuBuffer> buffer)
{
	vkUnmapMemory(device, buffer->memory);
}

//�o�b�t�@�̐ݒ�𒼐ڎw�肵�āA�o�b�t�@���쐬����
std::shared_ptr<GpuBuffer> GpuBufferFactory::Create(VkDeviceSize bufferSize, const void* srcPtr
	, VkBufferUsageFlags usage, VkMemoryPropertyFlags property)
{
	//���̍\���̂�Ԃ�
	std::shared_ptr<GpuBuffer> buffer = std::make_shared<GpuBuffer>(shared_from_this());

	//�܂��쐬����o�b�t�@��gpu���[�J���̃������[���ǂ����𒲂ׂ�
	if (property & 1)
	{
		//�X�e�[�W���O�o�b�t�@��p�ӂ���
		GpuBuffer stagingBuffer(shared_from_this());

		builder->Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			, stagingBuffer.buffer, stagingBuffer.memory);
		
		//���[�J���̃o�b�t�@���쐬����
		builder->Create(bufferSize, usage, property, buffer->buffer, buffer->memory);

		//�X�e�[�W���O�o�b�t�@�Ƀf�[�^���R�s�[����
		memoryMap(buffer);
		copyMemory(bufferSize, srcPtr, stagingBuffer);
		memoryUnMap(buffer);
		
		//�X�e�[�W���O�o�b�t�@�̃f�[�^�����[�J���̃o�b�t�@�ɃR�s�[����
		copyBuffer(bufferSize, stagingBuffer, buffer);
	}
	else
	{
		//�X�e�[�W���O�o�b�t�@�͕K�v�Ȃ�


		//�o�b�t�@��p�ӂ���
		builder->Create(bufferSize, usage, property, buffer->buffer, buffer->memory);
		
		//�������̃f�[�^���o�b�t�@�ɃR�s�[����
		memoryMap(buffer);
		copyMemory(bufferSize, srcPtr, buffer);
	}

	return buffer;
}

//���炩���ߐݒ肳�ꂽ�o�b�t�@�̐ݒ�Ńo�b�t�@���쐬����
std::shared_ptr<GpuBuffer> GpuBufferFactory::Create(VkDeviceSize bufferSize, const void* srcPtr
	, BufferUsage usage, BufferTransferType transferType)
{
	//���̍\���̂�Ԃ�
	std::shared_ptr<GpuBuffer> buffer = std::make_shared<GpuBuffer>(shared_from_this());

	if (usage == BufferUsage::VERTEX || usage == BufferUsage::INDEX ||
		usage == BufferUsage::UIVERTEX || usage == BufferUsage::LOCALSTORAGE)
	{
		//�X�e�[�W���O�o�b�t�@��p�ӂ���
		GpuBuffer staging(shared_from_this());

		builder->Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging.buffer, staging.memory);

		//���[�J���̃o�b�t�@���쐬����
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);

		memoryMap(staging); //�X�e�[�W���O�o�b�t�@�̃��������}�b�v����
		//�f�[�^���X�e�[�W���O�o�b�t�@�Ƀf�[�^���R�s�[����
		copyMemory(bufferSize, srcPtr, staging);
		memoryUnMap(staging); //�X�e�[�W���O�o�b�t�@�̃��������A���}�b�v����

		//�X�e�[�W���O�o�b�t�@�̃f�[�^�����[�J���̃o�b�t�@�ɃR�s�[����
		copyBuffer(bufferSize, staging, buffer);
	}
	else
	{
		//�X�e�[�W���O�o�b�t�@�͕K�v�Ȃ�

		//�o�b�t�@��p�ӂ���
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);

		//�������̃f�[�^���o�b�t�@�ɃR�s�[����
		//�o�b�t�@�[�̓}�b�v�����܂܂ŁAcpu������A�N�Z�X�ł���悤�ɂ���
		memoryMap(buffer);
		copyMemory(bufferSize, srcPtr, buffer);
	}

	return buffer;
}

std::shared_ptr<GpuBuffer> GpuBufferFactory::Create(VkDeviceSize bufferSize, BufferUsage usage
	, BufferTransferType transferType)
{
	//���̍\���̂�Ԃ�
	std::shared_ptr<GpuBuffer> buffer = std::make_shared<GpuBuffer>(shared_from_this());

	if (usage == BufferUsage::VERTEX || usage == BufferUsage::INDEX ||
		usage == BufferUsage::UIVERTEX || usage == BufferUsage::LOCALSTORAGE)
	{
		//�X�e�[�W���O�o�b�t�@��p�ӂ���
		GpuBuffer staging(shared_from_this());

		builder->Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging.buffer, staging.memory);

		//���[�J���̃o�b�t�@���쐬����
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);
	}
	else
	{
		//�X�e�[�W���O�o�b�t�@�͕K�v�Ȃ�

		//�o�b�t�@��p�ӂ���
		builder->Create(bufferSize, convertUsageFlagBits(usage, transferType)
			, convertMemoryPropertyFlagBits(usage),
			buffer->buffer, buffer->memory);
	}

	return buffer;
}

//�R�}���h�o�b�t�@�[���쐬����
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

//�x���j�����X�g�Ƀo�b�t�@��ǉ�����
void GpuBufferFactory::addDefferedDestruct(VkBuffer& buffer, VkDeviceMemory& memory)
{
	destructList[frameIndex].push_back({ buffer,memory });
}

void GpuBufferFactory::addDefferedDestruct(VkCommandBuffer& commandBuffer)
{
	destructListCommand[frameIndex].push_back(commandBuffer);
}

//�o�b�t�@��j������
void GpuBufferFactory::resourceDestruct()
{
	//���ۂɃo�b�t�@��j������
	for (auto& bufferMemory : destructList[frameIndex])
	{
		vkDestroyBuffer(device, bufferMemory.first, nullptr);
		vkFreeMemory(device, bufferMemory.second, nullptr);
	}

	for (auto& command : destructListCommand[frameIndex])
	{
		vkFreeCommandBuffers(device, commandPool, 1, &command);
	}

	//�t���[���C���f�b�N�X���X�V����
	frameIndex = (frameIndex == 0) ? 1 : 0;
}