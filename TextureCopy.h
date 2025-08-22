#pragma once

#include"GpuBufferFactory.h"
#include"CommandBufferFactory.h"

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
		std::cout << "TextureCopy :: デストラクタ" << std::endl;
	}

	std::shared_ptr<TextureCopy> initProperty()
	{
		property.initProperty();

		return shared_from_this();
	}

	//コピーするビューのタイプを指定する
	std::shared_ptr<TextureCopy> withAspectMask(const VkImageAspectFlagBits& flag);

	//ミップマップレベルを指定する
	std::shared_ptr<TextureCopy> withSrcMipmapLevel(const uint32_t& mipmapLevel);
	
	std::shared_ptr<TextureCopy> withDstMipmapLevel(const uint32_t& mipmapLevel);

	//ベースレイヤーを指定する
	std::shared_ptr<TextureCopy> withSrcLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//レイヤー数を指定する
	std::shared_ptr<TextureCopy> withDstLayerRange(const uint32_t& baseLayer,const uint32_t& layerCount);

	//テクスチャサイズを指定する
	std::shared_ptr<TextureCopy> withSize(const uint32_t& width, const uint32_t& height);

	//コピー元のテクスチャを指定する
	std::shared_ptr<TextureCopy> withSrcTexture(const std::weak_ptr<Texture> src);

	//コピー先のテクスチャを指定する
	std::shared_ptr<TextureCopy> withDstTexture(const std::weak_ptr<Texture> dst);

	//コマンドバッファを指定する
	std::shared_ptr<TextureCopy> withCommandBuffer(const std::weak_ptr<CommandBuffer> command);

	//プロパティを返す
	const TexCopyProperty& Build()
	{
		return property;
	}

	std::shared_ptr<Texture> Copy(const TexCopyProperty& property);
};