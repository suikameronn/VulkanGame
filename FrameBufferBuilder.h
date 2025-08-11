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

class FrameBufferBuilder
{
private:

	VkDevice device;

	FrameBufferProperty property;

public:

	FrameBufferBuilder(VkDevice& d);

	FrameBufferBuilder initProperty();

	FrameBufferProperty Build();

	void Create(VkFramebuffer& frameBuffer, const FrameBufferProperty& property);

	//フレームバッファの幅と高さを設定する
	FrameBufferBuilder withWidthHeight(const uint32_t& width, const uint32_t& height);

	//レンダーパスを設定する
	FrameBufferBuilder withRenderPass(const std::shared_ptr<RenderPass> renderPass);

	//VkImageViewを積み上げる
	FrameBufferBuilder addViewAttachment(const std::shared_ptr<Texture> texture);
	FrameBufferBuilder addViewAttachment(const std::shared_ptr<Texture> texture, const uint32_t baseViewIndex, const uint32_t viewCount);

	//レイヤー数を設定する
	FrameBufferBuilder withLayerCount(const uint32_t& layerCount);
};