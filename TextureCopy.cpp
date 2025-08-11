#include"TextureFactory.h"

#include"TextureCopy.h"

//�R�s�[����r���[�̃^�C�v���w�肷��
TextureCopy& TextureCopy::withAspectMask(const VkImageAspectFlagBits& flag)
{
	property.copyInfo.srcSubresource.aspectMask = flag;
	property.copyInfo.dstSubresource.aspectMask = flag;

	return *this;
}

//�~�b�v�}�b�v���x�����w�肷��
TextureCopy& TextureCopy::withMipmapLevel(const uint32_t& mipmapLevel)
{
	property.copyInfo.srcSubresource.mipLevel = mipmapLevel;
	property.copyInfo.dstSubresource.mipLevel = mipmapLevel;

	return *this;
}

//�x�[�X���C���[���w�肷��
TextureCopy& TextureCopy::withBaseLayer(const uint32_t& baseLayer)
{
	property.copyInfo.srcSubresource.baseArrayLayer = baseLayer;
	property.copyInfo.dstSubresource.baseArrayLayer = baseLayer;

	return *this;
}

//���C���[�����w�肷��
TextureCopy& TextureCopy::withLayerCount(const uint32_t& layerCount)
{
	property.copyInfo.srcSubresource.layerCount = layerCount;
	property.copyInfo.dstSubresource.layerCount = layerCount;

	return *this;
}

//�e�N�X�`���T�C�Y���w�肷��
TextureCopy& TextureCopy::withSize(const uint32_t& width, const uint32_t& height)
{
	property.copyInfo.extent = { width,height,1 };

	return *this;
}

//�R�s�[���̃e�N�X�`�����w�肷��
TextureCopy& TextureCopy::withSrcTexture(const std::shared_ptr<Texture> src)
{
	property.src = src;

	return *this;
}

//�R�s�[��̃e�N�X�`�����w�肷��
TextureCopy& TextureCopy::withDstTexture(const std::shared_ptr<Texture> dst)
{
	property.dst = dst;

	return *this;
}

//�R�}���h�o�b�t�@���w�肷��
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