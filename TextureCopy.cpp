#include"TextureFactory.h"

#include"TextureCopy.h"

//�R�s�[����r���[�̃^�C�v���w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withAspectMask(const VkImageAspectFlagBits& flag)
{
	property.copyInfo.srcSubresource.aspectMask = flag;
	property.copyInfo.dstSubresource.aspectMask = flag;

	return shared_from_this();
}

//�~�b�v�}�b�v���x�����w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withSrcMipmapLevel(const uint32_t& mipmapLevel)
{
	property.copyInfo.srcSubresource.mipLevel = mipmapLevel;

	return shared_from_this();
}

std::shared_ptr<TextureCopy> TextureCopy::withDstMipmapLevel(const uint32_t& mipmapLevel)
{
	property.copyInfo.dstSubresource.mipLevel = mipmapLevel;

	return shared_from_this();
}

//�x�[�X���C���[���w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withSrcLayerRange(const uint32_t& baseLayer, const uint32_t& layerCount)
{
	property.copyInfo.srcSubresource.baseArrayLayer = baseLayer;
	property.copyInfo.srcSubresource.layerCount = layerCount;

	return shared_from_this();
}

//���C���[�����w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withDstLayerRange(const uint32_t& baseLayer, const uint32_t& layerCount)
{
	property.copyInfo.dstSubresource.baseArrayLayer = baseLayer;
	property.copyInfo.dstSubresource.layerCount = layerCount;

	return shared_from_this();
}

//�e�N�X�`���T�C�Y���w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withSize(const uint32_t& width, const uint32_t& height)
{
	property.copyInfo.extent = { width,height,1 };

	return shared_from_this();
}

//�R�s�[���̃e�N�X�`�����w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withSrcTexture(const std::weak_ptr<Texture> src)
{
	property.src = src;

	return shared_from_this();
}

//�R�s�[��̃e�N�X�`�����w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withDstTexture(const std::weak_ptr<Texture> dst)
{
	property.dst = dst;

	return shared_from_this();
}

//�R�}���h�o�b�t�@���w�肷��
std::shared_ptr<TextureCopy> TextureCopy::withCommandBuffer(const std::weak_ptr<CommandBuffer> command)
{
	property.commandBuffer = command;

	return shared_from_this();
}

std::shared_ptr<Texture> TextureCopy::Copy(const TexCopyProperty& prop)
{
	if(prop.commandBuffer.expired() || prop.src.expired() || prop.dst.expired())
	{
		throw std::runtime_error("TextureCopy : weak_ptr");
	}

	vkCmdCopyImage(prop.commandBuffer.lock()->commandBuffer, prop.src.lock()->image
		, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, prop.dst.lock()->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &prop.copyInfo);

	return prop.dst.lock();
}