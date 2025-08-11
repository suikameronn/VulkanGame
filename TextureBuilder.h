#pragma once

#include<unordered_map>
#include<string>

#include"VulkanCore.h"
#include"GpuBufferFactory.h"

#include "tiny_gltf.h"

#define RGBA 4

struct Texture;

struct TextureImageProperty
{
	VkImageCreateInfo info;//VkImageの作成情報
	VkImageLayout finalLayout; //最終的なテクスチャのレイアウト
	VkMemoryPropertyFlags memProperty;

	size_t pixelSize;
	void* pixels;

	void initProperty()
	{
		info = VkImageCreateInfo{};

		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.extent.depth = 1;
		info.imageType = VK_IMAGE_TYPE_2D;

		finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		pixelSize = 0;

		if (!pixels)
		{
			delete[] pixels;
		}

		pixels = nullptr;
	}
};

struct TextureViewProperty
{
	VkImageViewCreateInfo info;

	void initProperty()
	{
		info = VkImageViewCreateInfo{};

		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.subresourceRange.baseArrayLayer = 0;
	}
};

struct TextureSamplerProperty
{
	VkSamplerCreateInfo info; //サンプラーの作成情報

	void initProperty()
	{
		info = VkSamplerCreateInfo{};

		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.minLod = 0.0f;
		info.maxLod = 1.0f;
		info.mipLodBias = 0.0f;
		info.maxAnisotropy = 1.0f;
		info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	}
};

struct TextureProperty
{
	TextureImageProperty image;
	std::vector<TextureViewProperty> viewArray;
	TextureSamplerProperty sampler;

	void initProperty()
	{
		image.initProperty();

		viewArray.clear();

		sampler.initProperty();
	}
};

class TextureBuilder
{
private:

	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	std::shared_ptr<GpuBufferFactory> bufferFactory;

	//各種プロパティ
	TextureProperty property;

	TextureImageProperty image;
	TextureViewProperty view;
	TextureSamplerProperty sampler;

	//vkCreateImageを呼び出す
	void createVkImageAndMemory(const TextureImageProperty& info, VkImage& image, VkDeviceMemory& memory);

	//バッファからVkImageに画像データをコピー
	void copyBufferToImage(const std::shared_ptr<GpuBuffer> stagingBuffer, VkImage& image
		, const TextureImageProperty& imageProperty);

	//ミップマップレベルを作成
	void generateMipmaps(VkImage image, const TextureImageProperty& imageProperty);

	//gpu上に画像データを展開
	void createImage(const size_t& pixelSize, const void* pixels
		, const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory);
	void createImage(const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory);

	//このテクスチャのビューを作成
	void createImageView(const TextureViewProperty& viewProperty, VkImage& image, VkImageView& view);

	//サンプラーの作成
	void createSampler(const TextureSamplerProperty& samplerProperty,VkSampler& sampler);

	//外部の画像を読み込む
	void loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
		, unsigned char* pixels);
	void loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
		, float* pixels);

public:

	TextureBuilder(std::shared_ptr<VulkanCore> core,std::shared_ptr<GpuBufferFactory> buffer);

	//プロパティを取得する
	TextureProperty Build();

	void Create(const uint32_t& texChannel, const unsigned char* pixels, const TextureProperty& property
		, VkImage& image, VkDeviceMemory& memory, std::vector<VkImageView>& viewArray, VkSampler& sampler);
	
	void Create(const TextureProperty& property
		, VkImage& image, VkDeviceMemory& memory, std::vector<VkImageView>& viewArray, VkSampler& sampler);

	//スワップチェーン用
	void Create(const TextureProperty& property, VkImage& image, VkDeviceMemory& memory, std::vector<VkImageView>& viewArray);

	//プロパティの初期化
	TextureBuilder& initProperty();

	//テクスチャサイズの設定
	TextureBuilder& withWidthHeight(const uint32_t& width, const uint32_t& height);
	TextureBuilder& withWidthHeight(const uint32_t& width, const uint32_t& height,const uint32_t& mipmapLevel);

	//画像ファイルからピクセルを読み取る
	TextureBuilder& withImageFile(const std::string filePath);
	TextureBuilder& withImageFile(const std::string filePath, const uint32_t miplevels);

	//フォーマットを設定
	TextureBuilder& withFormat(const VkFormat& format);

	//VkImageTypeを設定
	TextureBuilder& withImageType(const VkImageType& type);

	//VkImageFlagを設定(SamplerCubeの作成時などに使用)
	TextureBuilder& withImageFlag(const VkImageCreateFlagBits& flag);

	//マルチサンプリング数の設定
	TextureBuilder& withNumSamples(const VkSampleCountFlagBits& numSamples);

	//ピクセルの配置を設定
	TextureBuilder& withTiling(const VkImageTiling& tiling);

	//テクスチャのバッファの使い道を設定
	TextureBuilder& withUsage(const VkImageUsageFlags& usage);

	//メモリ配置を設定
	TextureBuilder& withMemoryProperty(const VkMemoryPropertyFlags& prop);

	//初期のテクスチャのレイアウト
	TextureBuilder& withInitialLayout(const VkImageLayout& layout);

	//最終的なテクスチャのレイアウト
	TextureBuilder& withFinalLayout(const VkImageLayout& layout);

	//テクスチャのレイヤー数を設定する
	TextureBuilder& withLayerCount(const uint32_t& layerCount);

	//ビューのタイプを設定する
	TextureBuilder& withViewType(const VkImageViewType& type);

	//ビューがアクセスできるデータを設定
	TextureBuilder& withViewAccess(const VkImageAspectFlags& flag);

	//ビューがアクセスするレイヤーの範囲を指定する
	TextureBuilder& withTargetLayer(const uint32_t baseLayer,const uint32_t tagetLayerCount);

	//ビューがアクセスするミップマップレベルを指定する
	TextureBuilder& withTargetMipmapLevel(const uint32_t baseMipmapLevel, const uint32_t levelCount);

	//ビューを積み上げる
	TextureBuilder& addView();

	//サンプラーのミップマップレベル間の補間方法を設定する
	TextureBuilder& withMipMapMode(const VkSamplerMipmapMode& mode);

	//テクスチャの境界部分の処理を設定
	TextureBuilder& withAddressMode(const VkSamplerAddressMode& mode);

	//テクスチャの拡大時の補間方法を設定する
	TextureBuilder& withMagFilter(const VkFilter& filter);

	//テクスチャの縮小時の補間方法を設定する
	TextureBuilder& withMinFilter(const VkFilter& filter);

	//画像のレイアウトを変更する
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout
		, uint32_t mipLevels, uint32_t layerCount);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout
		, uint32_t mipLevels, uint32_t layerCount, std::shared_ptr<CommandBuffer> commandBuffer);
};