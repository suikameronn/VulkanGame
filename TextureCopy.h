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

	//コピーするビューのタイプを指定する
	TextureCopy& withAspectMask(const VkImageAspectFlagBits& flag);

	//ミップマップレベルを指定する
	TextureCopy& withSrcMipmapLevel(const uint32_t& mipmapLevel);
	
	TextureCopy& withDstMipmapLevel(const uint32_t& mipmapLevel);

	//ベースレイヤーを指定する
	TextureCopy& withSrcLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//レイヤー数を指定する
	TextureCopy& withDstLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//テクスチャサイズを指定する
	TextureCopy& withSize(const uint32_t& width, const uint32_t& height);

	//コピー元のテクスチャを指定する
	TextureCopy& withSrcTexture(const std::shared_ptr<Texture> src);

	//コピー先のテクスチャを指定する
	TextureCopy& withDstTexture(const std::shared_ptr<Texture> dst);

	//コマンドバッファを指定する
	TextureCopy& withCommandBuffer(const std::shared_ptr<CommandBuffer> command);

	//プロパティを返す
	TexCopyProperty Build()
	{
		return property;
	}

	std::shared_ptr<Texture> Copy(const TexCopyProperty& property);
};