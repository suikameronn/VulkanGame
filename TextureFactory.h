#pragma once

#include"TextureBuilder.h"

enum class TexturePattern
{
	NORMAL,
	CUBEMAP,
	CALC_CUBEMAP
};

class TextureFactory : public std::enable_shared_from_this<TextureFactory>
{
private:

	//フレームインデックス
	uint32_t frameIndex;

	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	//ビルダー
	std::shared_ptr<TextureBuilder> builder;

	//外部の画像ファイルを読み込んだ場合はここにTextureを記録する
	std::unordered_map<std::string, std::weak_ptr<Texture>> textureStorage;

	//破棄予定リスト
	std::array<std::list<VkImage>, 2> destructListImage;
	std::array<std::list<VkDeviceMemory>, 2> destructListMemory;
	std::array<std::list<VkImageView>, 2> destructListView;
	std::array<std::list<VkSampler>, 2> destructListSampler;

	//既定のパターンからプロパティに変換
	TextureProperty convertPattern(const uint32_t& width, const uint32_t& height
		, const TexturePattern& pattern);
	//既定のパターンからプロパティに変換
	TextureProperty convertPattern(const TexturePattern& pattern);

public:

	TextureFactory(std::shared_ptr<VulkanCore> core, std::shared_ptr<TextureBuilder> b);

	//ビルダーを取得
	std::shared_ptr<TextureBuilder> getBuilder()
	{
		return builder;
	}

	//プリセットのプロパティを取得
	TextureProperty getPreset(const TexturePattern& pattern);

	//外部の画像ファイルからテクスチャを作る場合を分ける
	std::shared_ptr<Texture> Create(const std::string& filePath, TextureProperty& property);

	//外部の画像ファイルからテクスチャを作る場合を分ける
	//既定のプロパティを使う
	std::shared_ptr<Texture> Create(const std::string& filePath, const uint32_t& width
		, const uint32_t& height, const TexturePattern& pattern);

	//画像データを配列上で受け取る
	std::shared_ptr<Texture> Create(const uint32_t& texChannel, const unsigned char* pixels
		, const uint32_t& width, const uint32_t& height, const TexturePattern& pattern);

	//画像データは入れずに、テクスチャバッファのみを作る
	std::shared_ptr<Texture> Create(TextureProperty& property);
	std::shared_ptr<Texture> Create(const uint32_t& width, const uint32_t& height
		, const TexturePattern& pattern);

	//スワップチェーン用
	std::shared_ptr<Texture> ImageViewCreate(TextureProperty& property);
	std::shared_ptr<Texture> ViewCreate(TextureProperty& property, VkImage& image);

	//遅延破棄リストにリソースを追加する
	void addDefferedDestruct(VkImage& image, VkDeviceMemory& memory
		, VkImageView& view, VkSampler& sampler);

	//リソースを破棄する
	void resourceDestruct();
};

struct Texture
{
	//テクスチャの画像データ
	VkImage image;
	VkDeviceMemory memory;
	//画像のビュー
	VkImageView view;
	//画像のサンプラー テクスチャの境界の設定など
	VkSampler sampler;

	//各種プロパティを記録
	TextureProperty property;

	std::shared_ptr<TextureFactory> factory;

	Texture(std::shared_ptr<TextureFactory> f)
	{
		image = nullptr;
		memory = nullptr;
		view = nullptr;
		sampler = nullptr;

		property = TextureProperty{};

		factory = f;
	}

	~Texture()
	{
		factory->addDefferedDestruct(image, memory, view, sampler);
	}
};