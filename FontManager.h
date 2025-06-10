#pragma once

#include<iostream>
#include<unordered_map>
#include<fstream>

#include<vulkan/vulkan.h>

#include"ft2build.h"
#include FT_FREETYPE_H
#include"utf8.h"

#include"MaxRectPacking.h"

#define PADDING_PIXEL 2

struct CharFont
{
	glm::vec2 size;//ビットマップのサイズ
	glm::vec2 bearing;//ベースラインからのオフセット
	float advance;//次の文字までの進み幅
	glm::vec2 uvMin;//アトラス内のuv座標
	glm::vec2 uvMax;//アトラス内のuv座標
	float fontHeight;//改行時に下げるべきピクセル値

	Rect rect;//アトラステクスチャ内のビットマップがある領域

	void uvSet(float width, float height)
	{
		uvMin = glm::vec2(rect.x / width, rect.y / height);
		uvMax = glm::vec2((rect.x + rect.width) / width, (rect.y + rect.height) / height);
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

	static FontManager* GetInstance()
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

	//フォント全体の高さを取得
	const float getFontHeight();

	std::shared_ptr<ImageData> getTexture() { return atlasTexture; }
	VkDescriptorSet& getDescriptorSet() { return atlasTexDescriptorSet; }

	//一つの文字のフォント画像を取得
	void getCharFont(const std::vector<uint32_t>& utf8Codes, std::vector<CharFont>& charFonts);
};