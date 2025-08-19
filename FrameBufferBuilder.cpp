#include"FrameBufferBuilder.h"

FrameBufferBuilder::FrameBufferBuilder(VkDevice& d)
{
	device = d;
}

std::shared_ptr<FrameBufferBuilder> FrameBufferBuilder::initProperty()
{
	property.initProperty();

	return shared_from_this();
}

FrameBufferProperty FrameBufferBuilder::Build()
{
	return property;
}

void FrameBufferBuilder::Create(VkFramebuffer& frameBuffer, const FrameBufferProperty& property)
{
	VkFramebufferCreateInfo info = property.info;

	info.attachmentCount = static_cast<uint32_t>(property.texture.size());

	std::vector<VkImageView> attachments(property.getAttachmentSize());

	int index = 0;
	for(int i = 0;i < property.texture.size(); i++)
	{
		for (int j = 0; j < property.targetLayerIndex[i].size(); j++)
		{
			attachments[index] = property.texture[i]->viewArray[property.targetLayerIndex[i][j]];

			index++;
		}
	}

	info.pAttachments = attachments.data();

	if (vkCreateFramebuffer(device, &info, nullptr, &frameBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("FrameBufferBuilder : Create() : フレームバッファの作成に失敗");
	}
}

//フレームバッファの幅と高さを設定する
std::shared_ptr<FrameBufferBuilder> FrameBufferBuilder::withWidthHeight(const uint32_t& width, const uint32_t& height)
{
	property.info.width = width;
	property.info.height = height;

	return shared_from_this();
}

//レンダーパスを設定する
std::shared_ptr<FrameBufferBuilder> FrameBufferBuilder::withRenderPass(const std::shared_ptr<RenderPass> renderPass)
{
	property.renderPass = renderPass;
	property.info.renderPass = renderPass->renderPass;

	return shared_from_this();
}

//VkImageViewを積み上げる、この時テクスチャのどのレイヤーのビューを対象にするかも設定する
std::shared_ptr<FrameBufferBuilder> FrameBufferBuilder::addViewAttachment(const std::shared_ptr<Texture> texture)
{
	property.targetLayerIndex.push_back({ 0 });
	property.texture.push_back(texture);

	return shared_from_this();
}

std::shared_ptr<FrameBufferBuilder> FrameBufferBuilder::addViewAttachment(const std::shared_ptr<Texture> texture, const uint32_t baseViewIndex, const uint32_t viewCount)
{
	std::vector<uint32_t> indexArray(viewCount);
	for (int i = 0; i < viewCount; i++)
	{
		indexArray[i] = baseViewIndex + i;
	}

	property.targetLayerIndex.push_back(indexArray);
	property.texture.push_back(texture);

	return shared_from_this();
}

//レイヤー数を設定する
std::shared_ptr<FrameBufferBuilder> FrameBufferBuilder::withLayerCount(const uint32_t& layerCount)
{
	property.info.layers = layerCount;

	return shared_from_this();
}