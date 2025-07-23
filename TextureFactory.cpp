#include"TextureFactory.h"

TextureFactory::TextureFactory(VkDevice& d, std::shared_ptr<TextureBuilder> b)
{
	builder = b;

	device = d;
}

//プリセットのプロパティを取得
TextureProperty TextureFactory::getPreset(const TexturePattern& pattern)
{
	return convertPattern(pattern);
}

TextureProperty TextureFactory::convertPattern(const uint32_t& width, const uint32_t& height, const TexturePattern& pattern)
{
	if (pattern == TexturePattern::NORMAL)
	{
		//通常の3Dモデルなどに使われるテクスチャ

		return builder->initProperty()
			.withWidthHeight(width, height) //幅と高さ
			.withFormat(VK_FORMAT_R8G8B8A8_SRGB) //sRGBフォーマット
			.withNumSamples(VK_SAMPLE_COUNT_1_BIT) //サンプリング数
			.withTiling(VK_IMAGE_TILING_OPTIMAL) //タイル配置
			.withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT) //使用方法
			.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //デバイスローカルメモリ
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED) //初期レイアウト
			.withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) //最終レイアウト
			.withViewType(VK_IMAGE_VIEW_TYPE_2D) //ビュータイプ
			.withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT) //アスペクトフラグ
			.withLayerCount(1) //レイヤー数
			.withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR) //ミップマップモード
			.withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT) //アドレスモード
			.withMagFilter(VK_FILTER_LINEAR) //拡大時のフィルタ
			.withMinFilter(VK_FILTER_LINEAR) //縮小時のフィルタ
			.Build();

	}
	else if (pattern == TexturePattern::CUBEMAP)
	{
		//キューブマップやIBL用のテクスチャ

		return builder->initProperty()
			.withWidthHeight(width, height) //幅と高さ
			.withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			.withTiling(VK_IMAGE_TILING_OPTIMAL)
			.withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT
				| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			.withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			.withLayerCount(6) //キューブマップなので6面
			.withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			.withMagFilter(VK_FILTER_LINEAR)
			.withMinFilter(VK_FILTER_LINEAR)
			.Build();
	}
	else if (pattern == TexturePattern::CALC_CUBEMAP)
	{
		//キューブマップの計算用のテクスチャ
		return builder->initProperty()
			.withWidthHeight(width, height) //幅と高さ
			.withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			.withTiling(VK_IMAGE_TILING_OPTIMAL)
			.withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			.withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			.withLayerCount(1)
			.withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) //計算用なのでエッジで切る
			.withMagFilter(VK_FILTER_LINEAR)
			.withMinFilter(VK_FILTER_LINEAR)
			.Build();
	}
}

TextureProperty TextureFactory::convertPattern(const TexturePattern& pattern)
{
	if (pattern == TexturePattern::NORMAL)
	{
		//通常の3Dモデルなどに使われるテクスチャ

		return builder->initProperty()
			.withFormat(VK_FORMAT_R8G8B8A8_SRGB) //sRGBフォーマット
			.withNumSamples(VK_SAMPLE_COUNT_1_BIT) //サンプリング数
			.withTiling(VK_IMAGE_TILING_OPTIMAL) //タイル配置
			.withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT) //使用方法
			.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //デバイスローカルメモリ
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED) //初期レイアウト
			.withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) //最終レイアウト
			.withViewType(VK_IMAGE_VIEW_TYPE_2D) //ビュータイプ
			.withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT) //アスペクトフラグ
			.withLayerCount(1) //レイヤー数
			.withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR) //ミップマップモード
			.withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT) //アドレスモード
			.withMagFilter(VK_FILTER_LINEAR) //拡大時のフィルタ
			.withMinFilter(VK_FILTER_LINEAR) //縮小時のフィルタ
			.Build();

	}
	else if (pattern == TexturePattern::CUBEMAP)
	{
		//キューブマップやIBL用のテクスチャ

		return builder->initProperty()
			.withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			.withTiling(VK_IMAGE_TILING_OPTIMAL)
			.withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT
				| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			.withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			.withLayerCount(6) //キューブマップなので6面
			.withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			.withMagFilter(VK_FILTER_LINEAR)
			.withMinFilter(VK_FILTER_LINEAR)
			.Build();
	}
	else if (pattern == TexturePattern::CALC_CUBEMAP)
	{
		//キューブマップの計算用のテクスチャ
		return builder->initProperty()
			.withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			.withTiling(VK_IMAGE_TILING_OPTIMAL)
			.withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			.withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			.withLayerCount(1)
			.withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) //計算用なのでエッジで切る
			.withMagFilter(VK_FILTER_LINEAR)
			.withMinFilter(VK_FILTER_LINEAR)
			.Build();
	}
}

//外部の画像ファイルからテクスチャを作る場合を分ける
std::shared_ptr<Texture> TextureFactory::Create(const std::string& filePath, TextureProperty& property)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this());

	//ビルダーで画像ファイルからテクスチャを作成
	builder->Create(filePath, property, texture->image
		, texture->memory, texture->view, texture->sampler);

	textureStorage[filePath] = texture;

	return texture;
}

//外部の画像ファイルからテクスチャを作る場合を分ける
//既定のプリセットからプロパティを設定する
std::shared_ptr<Texture> TextureFactory::Create(const std::string& filePath, const uint32_t& width
	, const uint32_t& height, const TexturePattern& pattern)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this());

	TextureProperty property = convertPattern(width, height, pattern);

	//ビルダーで画像ファイルからテクスチャを作成
	builder->Create(property, texture->image
		, texture->memory, texture->view, texture->sampler);

	textureStorage[filePath] = texture;

	return texture;
}

//画像データを配列上で受け取る
std::shared_ptr<Texture> TextureFactory::Create(const uint32_t& texChannel, const unsigned char* pixels
	, const uint32_t& width, const uint32_t& height, const TexturePattern& pattern)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this());

	TextureProperty property = convertPattern(width, height, pattern);

	//ビルダーで画像ファイルからテクスチャを作成
	builder->Create(texChannel, pixels, property
		, texture->image, texture->memory, texture->view, texture->sampler);

	return texture;
}

//画像データは入れずに、テクスチャバッファのみを作る
std::shared_ptr<Texture> TextureFactory::Create(TextureProperty& property)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this());

	//ビルダーで画像ファイルからテクスチャを作成
	builder->Create(property, texture->image
		, texture->memory, texture->view, texture->sampler);

	return texture;
}

//画像データは入れずに、テクスチャバッファのみを作る
std::shared_ptr<Texture> TextureFactory::Create(const uint32_t& width, const uint32_t& height
	, const TexturePattern& pattern)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this());

	TextureProperty property = convertPattern(width, height, pattern);

	//ビルダーで画像ファイルからテクスチャを作成
	builder->Create(property, texture->image
		, texture->memory, texture->view, texture->sampler);

	return texture;
}

//スワップチェーン用の画像とビューを作成(サンプラーは作成しない)
std::shared_ptr<Texture> TextureFactory::ImageViewCreate(TextureProperty& property)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this());

	//ビルダーで画像ファイルからテクスチャを作成
	builder->Create(property, texture->image, texture->memory, texture->view);

	return texture;
}

//画像データは入れずに、テクスチャバッファのみを作る
std::shared_ptr<Texture> TextureFactory::ViewCreate(TextureProperty& property, VkImage& image)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this());

	texture->image = image;

	//ビルダーで画像からテクスチャを作成
	builder->Create(property, texture->image, texture->memory, texture->view);

	return texture;
}

//遅延破棄リストにリソースを追加する
void TextureFactory::addDefferedDestruct(VkImage& image, VkDeviceMemory& memory
	, VkImageView& view, VkSampler& sampler)
{
	destructListImage[frameIndex].push_back(image);
	destructListMemory[frameIndex].push_back(memory);
	destructListView[frameIndex].push_back(view);
	destructListSampler[frameIndex].push_back(sampler);
}

//リソースを破棄する
void TextureFactory::resourceDestruct()
{
	for (VkImage& image : destructListImage[frameIndex])
	{
		vkDestroyImage(device, image, nullptr);
	}

	for (VkDeviceMemory& memory : destructListMemory[frameIndex])
	{
		vkFreeMemory(device, memory, nullptr);
	}

	for (VkImageView& view : destructListView[frameIndex])
	{
		vkDestroyImageView(device, view, nullptr);
	}

	for (VkSampler& sampler : destructListSampler[frameIndex])
	{
		vkDestroySampler(device, sampler, nullptr);
	}

	frameIndex = (frameIndex == 0) ? 1 : 0;
}