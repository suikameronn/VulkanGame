#pragma once

#include"TextureBuilder.h"

#include"TextureCopy.h"

enum class TexturePattern
{
	NORMAL,
	CUBEMAP,
	CALC_CUBEMAP
};

class TextureFactory : public std::enable_shared_from_this<TextureFactory>
{
private:

	uint32_t count;

	//フレームインデックス
	uint32_t frameIndex;

	VkDevice device;

	//ビルダー
	std::shared_ptr<TextureBuilder> builder;

	//コピー
	std::shared_ptr<TextureCopy> copy;

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
	TextureProperty convertPattern(const std::string imageFilePath, const TexturePattern& pattern);

public:

	TextureFactory(VkDevice& d, std::shared_ptr<TextureBuilder> b);

	~TextureFactory()
	{
		for (int i = 0; i < 2; i++)
		{
			resourceDestruct();
			resourceDestruct();
		}

#ifdef _DEBUG
		std::cout << "TextureFactory :: デストラクタ" << std::endl;
#endif
	}

	//ビルダーを取得
	std::shared_ptr<TextureBuilder> getBuilder()
	{
		return builder;
	}

	//コピーを取得
	std::shared_ptr<TextureCopy> getCopy()
	{
		return copy;
	}

	//プリセットのプロパティを取得
	TextureProperty getPreset(const TexturePattern& pattern);

	//外部の画像ファイルからテクスチャを作る場合を分ける
	//既定のプロパティを使う
	std::shared_ptr<Texture> Create(const std::string& filePath, const TexturePattern& pattern);

	//画像データを配列上で受け取る
	std::shared_ptr<Texture> Create(const uint32_t& texChannel, const unsigned char* pixels
		, const uint32_t& width, const uint32_t& height, const TexturePattern& pattern);

	//画像データは入れずに、テクスチャバッファのみを作る
	std::shared_ptr<Texture> Create(const TextureProperty& property);
	std::shared_ptr<Texture> Create(const uint32_t& width, const uint32_t& height
		, const TexturePattern& pattern);

	std::shared_ptr<Texture> ImageViewCreate(const TextureProperty& property);
	std::shared_ptr<Texture> ImageCreate(const TextureProperty& property);
	std::shared_ptr<Texture> ViewCreate(const TextureProperty& property, VkImage& image);

	//遅延破棄リストにリソースを追加する
	void addDefferedDestruct(VkImage& image, VkDeviceMemory& memory
		, std::vector<VkImageView>& viewArray, VkSampler& sampler);

	//リソースを破棄する
	void resourceDestruct();
};

struct Texture
{
	uint32_t index;

	//テクスチャの画像データ
	VkImage image;
	VkDeviceMemory memory;
	//画像のビュー
	std::vector<VkImageView> viewArray;
	//画像のサンプラー テクスチャの境界の設定など
	VkSampler sampler;

	//各種プロパティを記録
	TextureProperty property;

	std::shared_ptr<TextureFactory> factory;

	Texture(std::shared_ptr<TextureFactory> f,uint32_t count)
	{
		image = nullptr;
		memory = nullptr;
		viewArray.clear();
		sampler = nullptr;

		property = TextureProperty{};

		factory = f;

		index = count;

		if (index == 85 || index == 86)
		{
			std::cout << "Stop" << std::endl;
		}
	}

	~Texture()
	{
		factory->addDefferedDestruct(image, memory, viewArray, sampler);

		std::cout << index << std::endl;
	}
};