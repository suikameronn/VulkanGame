#pragma once

#include"GpuBufferFactory.h"

struct Texture;

struct TexCopyProperty
{
	VkImageCopy copyInfo;

	std::weak_ptr<Texture> src;

	std::weak_ptr<Texture> dst;

	std::weak_ptr<CommandBuffer> commandBuffer;

	void initProperty()
	{
		copyInfo = VkImageCopy{};

		src.reset();
		dst.reset();

		commandBuffer.reset();
	};
};

class TextureCopy : public std::enable_shared_from_this<TextureCopy>
{
private:

	TexCopyProperty property;

public:

	TextureCopy()
	{
		property.initProperty();
	}


	~TextureCopy()
	{
		std::cout << "TextureCopy :: �f�X�g���N�^" << std::endl;
	}

	std::shared_ptr<TextureCopy> initProperty()
	{
		property.initProperty();

		return shared_from_this();
	}

	//�R�s�[����r���[�̃^�C�v���w�肷��
	std::shared_ptr<TextureCopy> withAspectMask(const VkImageAspectFlagBits& flag);

	//�~�b�v�}�b�v���x�����w�肷��
	std::shared_ptr<TextureCopy> withSrcMipmapLevel(const uint32_t& mipmapLevel);
	
	std::shared_ptr<TextureCopy> withDstMipmapLevel(const uint32_t& mipmapLevel);

	//�x�[�X���C���[���w�肷��
	std::shared_ptr<TextureCopy> withSrcLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//���C���[�����w�肷��
	std::shared_ptr<TextureCopy> withDstLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//�e�N�X�`���T�C�Y���w�肷��
	std::shared_ptr<TextureCopy> withSize(const uint32_t& width, const uint32_t& height);

	//�R�s�[���̃e�N�X�`�����w�肷��
	std::shared_ptr<TextureCopy> withSrcTexture(const std::weak_ptr<Texture> src);

	//�R�s�[��̃e�N�X�`�����w�肷��
	std::shared_ptr<TextureCopy> withDstTexture(const std::weak_ptr<Texture> dst);

	//�R�}���h�o�b�t�@���w�肷��
	std::shared_ptr<TextureCopy> withCommandBuffer(const std::weak_ptr<CommandBuffer> command);

	//�v���p�e�B��Ԃ�
	const TexCopyProperty& Build()
	{
		return property;
	}

	std::shared_ptr<Texture> Copy(const TexCopyProperty& property);
};