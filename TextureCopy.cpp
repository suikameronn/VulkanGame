#include"TextureFactory.h"

#include"TextureCopy.h"

//コピーするビューのタイプを指定する
TextureCopy& TextureCopy::withAspectMask(const VkImageAspectFlagBits& flag)
{
	property.copyInfo.srcSubresource.aspectMask = flag;
	property.copyInfo.dstSubresource.aspectMask = flag;

	return *this;
}

//ミップマップレベルを指定する
TextureCopy& TextureCopy::withMipmapLevel(const uint32_t& mipmapLevel)
{
	property.copyInfo.srcSubresource.mipLevel = mipmapLevel;
	property.copyInfo.dstSubresource.mipLevel = mipmapLevel;

	return *this;
}

//ベースレイヤーを指定する
TextureCopy& TextureCopy::withBaseLayer(const uint32_t& baseLayer)
{
	property.copyInfo.srcSubresource.baseArrayLayer = baseLayer;
	property.copyInfo.dstSubresource.baseArrayLayer = baseLayer;

	return *this;
}

//レイヤー数を指定する
TextureCopy& TextureCopy::withLayerCount(const uint32_t& layerCount)
{
	property.copyInfo.srcSubresource.layerCount = layerCount;
	property.copyInfo.dstSubresource.layerCount = layerCount;

	return *this;
}

//テクスチャサイズを指定する
TextureCopy& TextureCopy::withSize(const uint32_t& width, const uint32_t& height)
{
	property.copyInfo.extent = { width,height,1 };

	return *this;
}

//コピー元のテクスチャを指定する
TextureCopy& TextureCopy::withSrcTexture(const std::shared_ptr<Texture> src)
{
	property.src = src;

	return *this;
}

//コピー先のテクスチャを指定する
TextureCopy& TextureCopy::withDstTexture(const std::shared_ptr<Texture> dst)
{
	property.dst = dst;

	return *this;
}

//コマンドバッファを指定する
TextureCopy& TextureCopy::withCommandBuffer(const std::shared_ptr<CommandBuffer> command)
{
	property.commandBuffer = command;

	return *this;
}

std::shared_ptr<Texture> TextureCopy::Copy(const TexCopyProperty& prop)
{
	vkCmdCopyImage(prop.commandBuffer->commandBuffer, prop.src->image
		, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, prop.dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &prop.copyInfo);

	std::shared_ptr<Texture> dstTex = prop.dst;

	initProperty();

	return dstTex;
}