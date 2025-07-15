#pragma once

#include"VulkanCore.h"
#include"GpuRenderPassFactory.h"
#include"TextureFactory.h"

#include"FrameBufferBuilder.h"

struct FrameBufferProperty
{
	VkFramebufferCreateInfo info;

	std::vector<std::shared_ptr<Texture>> texture;
	std::shared_ptr<RenderPass> renderPass;

	void initProperty()
	{
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		texture.clear();

		renderPass = nullptr;
	}
};

class FrameBufferBuilder
{
private:

	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	FrameBufferProperty property;

public:

	FrameBufferBuilder(std::shared_ptr<VulkanCore> core);

	FrameBufferBuilder initProperty();

	FrameBufferProperty Build();

	void Create(VkFramebuffer& frameBuffer, const FrameBufferProperty& property);

	//�t���[���o�b�t�@�̕��ƍ�����ݒ肷��
	FrameBufferBuilder withWidthHeight(const uint32_t& width, const uint32_t& height);

	//�����_�[�p�X��ݒ肷��
	FrameBufferBuilder withRenderPass(const std::shared_ptr<RenderPass> renderPass);

	//VkImageView��ςݏグ��
	FrameBufferBuilder addViewAttachment(const std::shared_ptr<Texture> texture);

	//���C���[����ݒ肷��
	FrameBufferBuilder withLayerCount(const uint32_t& layerCount);
};