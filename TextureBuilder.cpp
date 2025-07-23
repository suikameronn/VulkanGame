#include"TextureBuilder.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"

TextureBuilder::TextureBuilder(std::shared_ptr<VulkanCore> core,std::shared_ptr<GpuBufferFactory> buffer)
{
	vulkanCore = core;
	bufferFactory = buffer;

	device = vulkanCore->getLogicDevice();

	initProperty();
}

//プロパティの初期化
TextureBuilder TextureBuilder::initProperty()
{
	property.initProperty();

	return *this;
}

//プリセットからプロパティを設定する
TextureBuilder TextureBuilder::setPreset(const TextureProperty& p)
{
	initProperty();

	property = p;

	return *this;
}

//テクスチャサイズの設定
TextureBuilder TextureBuilder::withWidthHeight(const uint32_t& width, const uint32_t& height)
{
	property.image.info.extent.width = width;
	property.image.info.extent.height = height;

	property.image.info.mipLevels = std::floor(std::log2(std::max(width, height))) + 1;
	property.view.info.subresourceRange.levelCount = property.image.info.mipLevels;

	return *this;
}

//テクスチャサイズの設定
TextureBuilder TextureBuilder::withWidthHeight(const uint32_t& width, const uint32_t& height, const uint32_t& mipmapLevel)
{
	property.image.info.extent.width = width;
	property.image.info.extent.height = height;

	property.image.info.mipLevels = mipmapLevel;
	property.view.info.subresourceRange.levelCount = property.image.info.mipLevels;

	return *this;
}

//フォーマットを設定
TextureBuilder TextureBuilder::withFormat(const VkFormat& format)
{
	property.image.info.format = format;
	property.view.info.format = format;

	return *this;
}

//マルチサンプリング数の設定
TextureBuilder TextureBuilder::withNumSamples(const VkSampleCountFlagBits& numSamples)
{
	property.image.info.samples = numSamples;

	return *this;
}

//ピクセルの配置を設定
TextureBuilder TextureBuilder::withTiling(const VkImageTiling& tiling)
{
	property.image.info.tiling = tiling;

	return *this;
}

//テクスチャのバッファの使い道を設定
TextureBuilder TextureBuilder::withUsage(const VkImageUsageFlags& usage)
{
	property.image.info.usage = usage;

	return *this;
}

//メモリ配置を設定
TextureBuilder TextureBuilder::withMemoryProperty(const VkMemoryPropertyFlags& prop)
{
	property.image.memProperty = prop;

	return *this;
}

//初期のテクスチャのレイアウト
TextureBuilder TextureBuilder::withInitialLayout(const VkImageLayout& layout)
{
	property.image.info.initialLayout = layout;

	return *this;
}

//最終的なテクスチャのレイアウト
TextureBuilder TextureBuilder::withFinalLayout(const VkImageLayout& layout)
{
	property.image.finalLayout = layout;

	return *this;
}


//ビューのタイプを設定する
TextureBuilder TextureBuilder::withViewType(const VkImageViewType& type)
{
	property.view.info.viewType = type;

	return *this;
}

//ビューがアクセスできるデータを設定
TextureBuilder TextureBuilder::withViewAccess(const VkImageAspectFlags& flag)
{
	property.view.info.subresourceRange.aspectMask = flag;

	return *this;
}

//テクスチャのレイヤー数を設定する
TextureBuilder TextureBuilder::withLayerCount(const uint32_t& layerCount)
{
	property.image.info.arrayLayers = layerCount;
	property.view.info.subresourceRange.layerCount = layerCount;

	return *this;
}


//サンプラーのミップマップレベル間の補間方法を設定する
TextureBuilder TextureBuilder::withMipMapMode(const VkSamplerMipmapMode& mode)
{
	property.sampler.info.mipmapMode = mode;

	return *this;
}

//テクスチャの境界部分の処理を設定
TextureBuilder TextureBuilder::withAddressMode(const VkSamplerAddressMode& mode)
{
	property.sampler.info.addressModeU = mode;
	property.sampler.info.addressModeV = mode;
	property.sampler.info.addressModeW = mode;

	return *this;
}

//テクスチャの拡大時の補間方法を設定する
TextureBuilder TextureBuilder::withMagFilter(const VkFilter& filter)
{
	property.sampler.info.magFilter = filter;

	return *this;
}

//テクスチャの縮小時の補間方法を設定する
TextureBuilder TextureBuilder::withMinFilter(const VkFilter& filter)
{
	property.sampler.info.minFilter = filter;

	return *this;
}

TextureProperty TextureBuilder::Build()
{
	return property;
}

//外部の画像を読み込む
void TextureBuilder::loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
	, uint32_t& texChannel,unsigned char* pixels)
{
	//通常のテクスチャの画像を読み込む
	int texWidth, texHeight, texChannels;
	unsigned char* srcPixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, 0);
	if (!pixels)
	{
		throw std::runtime_error("画像の読み込みの失敗: " + filePath);
	}
	width = static_cast<uint32_t>(texWidth);
	height = static_cast<uint32_t>(texHeight);
	texChannel = 4;

	if (texChannels == 4)//画像に透明度のチャンネルがある場合は、そのまま
	{
		pixels = new unsigned char[width * height * texChannels];
		memcpy(pixels, srcPixels, (width * height * texChannels) - 1);
	}
	else if (texChannels == 3)//画像に透明度のチャンネルが無い場合は、すべてのピクセルの透明度を255として
		//強引にチャンネルを4つに gpuでのテクスチャ作成の都合上
	{
		pixels = new unsigned char[width * height * 4];
		memset(pixels, 255, width * height * 4);

		for (uint32_t i = 0; i < width * height; i++)
		{
			pixels[i * 4] = srcPixels[i * 3];
			pixels[i * 4 + 1] = srcPixels[i * 3 + 1];
			pixels[i * 4 + 2] = srcPixels[i * 3 + 2];
		}
	}
	else if (texChannels == 1)
	{
		pixels = new unsigned char[width * height * 4];
		memset(pixels, 255, width * height * 4);

		for (uint32_t i = 0; i < width * height; i++)
		{
			pixels[i * 4] = srcPixels[i];
		}
	}

	stbi_image_free(srcPixels);
}

void TextureBuilder::loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
	, uint32_t& texChannel, float* pixels)
{
	//HDRのテクスチャの画像を読み込む
	int texWidth, texHeight, texChannels;
	pixels = stbi_loadf(filePath.c_str(), &texWidth, &texHeight, &texChannels, 0);
	if (!pixels)
	{
		throw std::runtime_error("画像の読み込みの失敗: " + filePath);
	}
	width = static_cast<uint32_t>(texWidth);
	height = static_cast<uint32_t>(texHeight);
	texChannel = static_cast<uint32_t>(texChannels);
}

//vkCreateImageを呼び出す
void TextureBuilder::createVkImageAndMemory(const TextureImageProperty& property, VkImage& image, VkDeviceMemory& memory)
{
	if (vkCreateImage(device, &property.info, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("TextureBuilder : createVkImageAndMemory() : テクスチャの作成に失敗");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vulkanCore->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
	{
		throw std::runtime_error("TextureBuilder : createVkImageAndMemory() : テクスチャのバッファの確保に失敗");
	}

	vkBindImageMemory(device, image, memory, 0);
}

//画像のレイアウトを変更する
void TextureBuilder::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout
	, uint32_t mipLevels, uint32_t layerCount)
{
	VkCommandBuffer commandBuffer = vulkanCore->beginSingleTimeCommandBuffer();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	//レイアウトを出力先に変更する
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	//レイアウトをレンダリングの出力先にする
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	//出力されたテクスチャをシェーダ上で利用できるようにする
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	//デプスイメージ用にする
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	//カラーアタッチメント用のレイアウトを送信用のレイアウトに変更する
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	//カラーアタッチメント用のレイアウトをシェーダで使うためのレイアウトにする
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	//カラーアタッチメント用のレイアウトを表示用のレイアウトにする
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		//パイプラインの処理のすべてが終了したとき
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	//表示用のレイアウトをカラーアタッチメント用のレイアウトにする
	else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vulkanCore->endSingleTimeCommandBuffer(commandBuffer);
}

//バッファからVkImageに画像データをコピー
void TextureBuilder::copyBufferToImage(const std::shared_ptr<GpuBuffer> staging, VkImage& image
	, const TextureImageProperty& imageProperty)
{
	VkCommandBuffer commandBuffer = vulkanCore->beginSingleTimeCommandBuffer();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = imageProperty.info.arrayLayers;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		imageProperty.info.extent.width,
		imageProperty.info.extent.height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, staging->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	vulkanCore->endSingleTimeCommandBuffer(commandBuffer);
}


//ミップマップレベルを作成
void TextureBuilder::generateMipmaps(VkImage image, const TextureImageProperty& imageProperty)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(vulkanCore->getPhysicalDevice(), imageProperty.info.format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
	{
		throw std::runtime_error("TextureBuilder : generateMipmaps() : フォーマットに適合していない");
	}

	VkCommandBuffer commandBuffer = vulkanCore->beginSingleTimeCommandBuffer();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = imageProperty.info.arrayLayers;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = imageProperty.info.extent.width;
	int32_t mipHeight = imageProperty.info.extent.height;

	for (uint32_t i = 1; i < imageProperty.info.arrayLayers; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr
			, 0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0,0,0 };
		blit.srcOffsets[1] = { mipWidth,mipHeight,1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = imageProperty.info.arrayLayers;
		blit.dstOffsets[0] = { 0,0,0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1,1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = imageProperty.info.arrayLayers;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = imageProperty.info.mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	vulkanCore->endSingleTimeCommandBuffer(commandBuffer);
}

//gpu上に画像データを展開
void TextureBuilder::createImage(const size_t& pixelSize, const void* pixels
	,const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory)
{
	VkDeviceSize bufferSize = imageProperty.info.extent.width * imageProperty.info.extent.height * pixelSize * RGBA;

	//ステージングバッファを用意
	std::shared_ptr<GpuBuffer> staging = bufferFactory->Create(bufferSize,pixels
		, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//コピー先のVkImageを作成
	createVkImageAndMemory(imageProperty, image, memory);

	//ステージングバッファに画像データを画像データをコピー
	bufferFactory->copyMemory(bufferSize, pixels, staging);

	//コピー先のVkImageのレイアウトを変更
	transitionImageLayout(image, imageProperty.info.format, imageProperty.info.initialLayout,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageProperty.info.mipLevels, imageProperty.info.arrayLayers);

	//ステージングバッファからVkImageに画像データをコピー
	copyBufferToImage(staging, image, imageProperty);

	//ミップマップレベルを作成
	generateMipmaps(image, imageProperty);
}

//gpu上に画像データを展開
void TextureBuilder::createImage(const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory)
{
	VkDeviceSize bufferSize = imageProperty.info.extent.width * imageProperty.info.extent.height * sizeof(unsigned char) * RGBA;

	//コピー先のVkImageを作成
	createVkImageAndMemory(imageProperty, image, memory);

	//コピー先のVkImageのレイアウトを変更
	transitionImageLayout(image, imageProperty.info.format, imageProperty.info.initialLayout,
		imageProperty.finalLayout, imageProperty.info.mipLevels, imageProperty.info.arrayLayers);
}

//このテクスチャのビューを作成
void TextureBuilder::createImageView(TextureViewProperty& viewProperty, VkImage& image,VkImageView& view)
{
	viewProperty.info.image = image;

	if (vkCreateImageView(device, &viewProperty.info, nullptr, &view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

//サンプラーの作成
void TextureBuilder::createSampler(const TextureSamplerProperty& samplerProperty, VkSampler& sampler)
{
	if (vkCreateSampler(device, &samplerProperty.info, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image sampler");
	}
}

void TextureBuilder::Create(const std::string& filePath, TextureProperty& property
	, VkImage& image,VkDeviceMemory& memory,VkImageView& view,VkSampler& sampler)
{
	//外部の画像ファイルからテクスチャを作る
	uint32_t texChannel;
	void* pixels = nullptr;
	size_t pixelSize = 0;

	if (!stbi_is_hdr(filePath.c_str()))
	{
		loadImageFile(filePath, property.image.info.extent.width, property.image.info.extent.height,
			texChannel, static_cast<unsigned char*>(pixels));

		pixelSize = sizeof(unsigned char);
	}
	else
	{
		loadImageFile(filePath, property.image.info.extent.width, property.image.info.extent.height,
			texChannel, static_cast<float*>(pixels));

		pixelSize = sizeof(float);
	}

	//gpu上に画像データを展開
	createImage(pixelSize, pixels, property.image, image, memory);

	//このテクスチャのビューを作成
	createImageView(property.view, image, view);

	//サンプラーの作成
	createSampler(property.sampler, sampler);

	//画像のレイアウトを指定のものに変更
	transitionImageLayout(image, property.image.info.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		property.image.finalLayout, property.image.info.mipLevels, property.image.info.arrayLayers);

	//画像ファイルのピクセルデータを開放
	delete[] pixels;
}

void TextureBuilder::Create(const uint32_t& texChannel, const unsigned char* pixels, TextureProperty& property
	, VkImage& image, VkDeviceMemory& memory, VkImageView& view, VkSampler& sampler)
{
	//ピクセルの配列からからテクスチャを作る
	size_t pixelSize = sizeof(unsigned char);

	//gpu上に画像データを展開
	createImage(pixelSize, pixels, property.image, image, memory);

	//このテクスチャのビューを作成
	createImageView(property.view, image, view);

	//サンプラーの作成
	createSampler(property.sampler, sampler);
}

void TextureBuilder::Create(TextureProperty& property
	, VkImage& image, VkDeviceMemory& memory, VkImageView& view, VkSampler& sampler)
{
	//gpu上に画像データを展開
	createImage(property.image, image, memory);

	//このテクスチャのビューを作成
	createImageView(property.view, image, view);

	//サンプラーの作成
	createSampler(property.sampler, sampler);
}

//スワップチェーン用
void TextureBuilder::Create(TextureProperty& property, VkImage& image
	,VkDeviceMemory& memory, VkImageView& view)
{
	property.view.info.subresourceRange.levelCount = 1;

	if(image == VK_NULL_HANDLE && memory == VK_NULL_HANDLE)
	{
		createImage(property.image, image, memory);
	}

	//スワップチェーンのビューを作成
	createImageView(property.view, image, view);
}