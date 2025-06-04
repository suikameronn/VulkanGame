#pragma once

#include<iostream>
#include<unordered_map>
#include<fstream>

#include<vulkan/vulkan.h>

#include"ft2build.h"
#include FT_FREETYPE_H
#include"utf8.h"

#include "unicode/utypes.h"
#include "unicode/ucnv.h"
#include "unicode/utrans.h"
#include "unicode/unum.h"
#include "unicode/localpointer.h"

#include"MaxRectPacking.h"



#define PADDING_PIXEL 2

struct CharFont
{
	glm::ivec2 size;//ビットマップのサイズ
	glm::vec2 bearing;//ベースラインからのオフセット
	uint32_t advance;//次の文字までの進み幅
	glm::vec2 uvMin;//アトラス内のuv座標
	glm::vec2 uvMax;//アトラス内のuv座標

	Rect rect;//アトラステクスチャ内のビットマップがある領域

	void uvSet(int width, int height)
	{
		float texWidth = static_cast<float>(width);
		float texHeight = static_cast<float>(height);

		uvMin = glm::vec2(rect.x / texWidth, rect.y / texHeight);
		uvMax = glm::vec2((rect.x + rect.width) / texWidth, (rect.y + rect.height) / texHeight);

		if (rect.rotated)
		{
			uvMin = glm::vec2(rect.x / texWidth, (rect.y + rect.height) / texHeight);
			uvMax = glm::vec2((rect.x + rect.width) / texWidth, rect.y / texHeight);
		}
	}
};

class FontManager
{
private:

	static FontManager* instance;

	std::unique_ptr<MaxRectPacking> texturePack;

	FontManager();

	FT_Library library;
	FT_Face face;
	FT_GlyphSlot slot;

	const std::string fontPath = "Font/NotoSansJP-Regular.ttf";
	const std::string characterPath = "Font/japaneseList.txt";

	std::shared_ptr<ImageData> atlasTexture;

	std::unordered_map<uint32_t, CharFont> fontMap;
	std::unordered_map<uint32_t, std::shared_ptr<ImageData>> bitmaps;
	
	//アトラステクスチャのバッファを示す
	VkDescriptorSet atlasTexDescriptorSet;

	//アトラステクスチャの作成
	void createAtlasTexture();
	//日本語の一覧を文字列として読み込む
	void loadJapaneseFile();
	//ビットマップの作成
	void createBitmap(const uint32_t c);
	//すべての文字のビットマップをアトラステクスチャにまとめる
	void packBitmapsToAtlas(int atlasWidth,int atlasHeight);

	//utf-8からshif-jisに変換
	std::string convUtf8ToShiftJis(std::string& utf);

	~FontManager();

public:

	static FontManager* GeInstance()
	{
		if (!instance)
		{
			instance = new FontManager();
		}

		return instance;
	}

	static void FinishInstance()
	{
		delete instance;
		instance = nullptr;
	}

	std::shared_ptr<ImageData> getTexture() { return atlasTexture; }
	VkDescriptorSet& getDescriptorSet() { return atlasTexDescriptorSet; }

	//一つの文字のフォント画像を取得
	std::vector<CharFont>& getCharFont(const std::string str);
};