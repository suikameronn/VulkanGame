#pragma once

#include"GpuBufferFactory.h"

struct Texture;

struct TexCopyProperty
{
	VkImageCopy copyInfo;

	std::shared_ptr<Texture> src;

	std::shared_ptr<Texture> dst;

	std::shared_ptr<CommandBuffer> commandBuffer;

	void initProperty()
	{
		copyInfo = VkImageCopy{};

		src.reset();
		dst.reset();

		commandBuffer.reset();
	};
};

class TextureCopy
{
private:

	TexCopyProperty property;

public:

	TextureCopy()
	{
		property.initProperty();
	}

	TextureCopy& initProperty()
	{
		property.initProperty();

		return *this;
	}

	//�R�s�[����r���[�̃^�C�v���w�肷��
	TextureCopy& withAspectMask(const VkImageAspectFlagBits& flag);

	//�~�b�v�}�b�v���x�����w�肷��
	TextureCopy& withSrcMipmapLevel(const uint32_t& mipmapLevel);
	
	TextureCopy& withDstMipmapLevel(const uint32_t& mipmapLevel);

	//�x�[�X���C���[���w�肷��
	TextureCopy& withSrcLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//���C���[�����w�肷��
	TextureCopy& withDstLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//�e�N�X�`���T�C�Y���w�肷��
	TextureCopy& withSize(const uint32_t& width, const uint32_t& height);

	//�R�s�[���̃e�N�X�`�����w�肷��
	TextureCopy& withSrcTexture(const std::shared_ptr<Texture> src);

	//�R�s�[��̃e�N�X�`�����w�肷��
	TextureCopy& withDstTexture(const std::shared_ptr<Texture> dst);

	//�R�}���h�o�b�t�@���w�肷��
	TextureCopy& withCommandBuffer(const std::shared_ptr<CommandBuffer> command);

	//�v���p�e�B��Ԃ�
	TexCopyProperty Build()
	{
		return property;
	}

	std::shared_ptr<Texture> Copy(const TexCopyProperty& property);
};