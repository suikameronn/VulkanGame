#pragma once

#include"VulkanCore.h"
#include"RenderPassFactory.h"
#include"TextureFactory.h"

struct FrameBufferProperty
{
	VkFramebufferCreateInfo info;

	std::vector<std::vector<uint32_t>> targetLayerIndex;
	std::vector<std::shared_ptr<Texture>> texture;
	std::shared_ptr<RenderPass> renderPass;

	void initProperty()
	{
		info = VkFramebufferCreateInfo{};

		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		texture.clear();
		
		targetLayerIndex.clear();

		renderPass = nullptr;
	}

	size_t getAttachmentSize() const
	{
		size_t size = 0;

		for (auto& index : targetLayerIndex)
		{
			size += index.size();
		}

		return size;
	}
};

class FrameBufferBuilder : public std::enable_shared_from_this<FrameBufferBuilder>
{
private:

	VkDevice device;

	FrameBufferProperty property;

public:

	FrameBufferBuilder(VkDevice& d);

	~FrameBufferBuilder()
	{
#ifdef _DEBUG
		std::cout << "FrameBufferBuilder :: �f�X�g���N�^" << std::endl;
#endif
	}

	std::shared_ptr<FrameBufferBuilder> initProperty();

	FrameBufferProperty Build();

	void Create(VkFramebuffer& frameBuffer, const FrameBufferProperty& property);

	//�t���[���o�b�t�@�̕��ƍ�����ݒ肷��
	std::shared_ptr<FrameBufferBuilder> withWidthHeight(const uint32_t& width, const uint32_t& height);

	//�����_�[�p�X��ݒ肷��
	std::shared_ptr<FrameBufferBuilder> withRenderPass(const std::shared_ptr<RenderPass> renderPass);

	//VkImageView��ςݏグ��
	std::shared_ptr<FrameBufferBuilder> addViewAttachment(const std::shared_ptr<Texture> texture);
	std::shared_ptr<FrameBufferBuilder> addViewAttachment(const std::shared_ptr<Texture> texture, const uint32_t baseViewIndex, const uint32_t viewCount);

	//���C���[����ݒ肷��
	std::shared_ptr<FrameBufferBuilder> withLayerCount(const uint32_t& layerCount);
};