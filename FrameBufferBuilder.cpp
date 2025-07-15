#include"FrameBufferBuilder.h"

FrameBufferBuilder::FrameBufferBuilder(std::shared_ptr<VulkanCore> core)
{
	vulkanCore = core;

	device = vulkanCore->getLogicDevice();

	initProperty();
}

FrameBufferBuilder FrameBufferBuilder::initProperty()
{
	property.initProperty();

	return *this;
}

FrameBufferProperty FrameBufferBuilder::Build()
{
	FrameBufferProperty p = property;

	initProperty();

	return p;
}

void FrameBufferBuilder::Create(VkFramebuffer& frameBuffer, const FrameBufferProperty& property)
{
	VkFramebufferCreateInfo info = property.info;
	info.attachmentCount = static_cast<uint32_t>(property.texture.size());

	std::vector<VkImageView> attachments(property.texture.size());
	for(int i = 0;i < property.texture.size(); i++)
	{
		attachments.push_back(property.texture[i]->view);
	}

	info.pAttachments = attachments.data();

	if (vkCreateFramebuffer(device, &info, nullptr, &frameBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("FrameBufferBuilder : Create() : フレームバッファの作成に失敗");
	}
}

//フレームバッファの幅と高さを設定する
FrameBufferBuilder FrameBufferBuilder::withWidthHeight(const uint32_t& width, const uint32_t& height)
{
	property.info.width = width;
	property.info.height = height;

	return *this;
}

//レンダーパスを設定する
FrameBufferBuilder FrameBufferBuilder::withRenderPass(const std::shared_ptr<RenderPass> renderPass)
{
	property.renderPass = renderPass;

	return *this;
}

//VkImageViewを積み上げる
FrameBufferBuilder FrameBufferBuilder::addViewAttachment(const std::shared_ptr<Texture> texture)
{
	property.texture.push_back(texture);

	return *this;
}

//レイヤー数を設定する
FrameBufferBuilder FrameBufferBuilder::withLayerCount(const uint32_t& layerCount)
{
	property.info.layers = layerCount;

	return *this;
}