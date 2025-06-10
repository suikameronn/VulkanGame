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
	glm::vec2 size;//�r�b�g�}�b�v�̃T�C�Y
	glm::vec2 bearing;//�x�[�X���C������̃I�t�Z�b�g
	float advance;//���̕����܂ł̐i�ݕ�
	glm::vec2 uvMin;//�A�g���X����uv���W
	glm::vec2 uvMax;//�A�g���X����uv���W
	float fontHeight;//���s���ɉ�����ׂ��s�N�Z���l

	Rect rect;//�A�g���X�e�N�X�`�����̃r�b�g�}�b�v������̈�

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
	
	//�A�g���X�e�N�X�`���̃o�b�t�@������
	VkDescriptorSet atlasTexDescriptorSet;

	//�A�g���X�e�N�X�`���̍쐬
	void createAtlasTexture();
	//���{��̈ꗗ�𕶎���Ƃ��ēǂݍ���
	void loadJapaneseFile();
	//�r�b�g�}�b�v�̍쐬
	void createBitmap(const uint32_t c);
	//���ׂĂ̕����̃r�b�g�}�b�v���A�g���X�e�N�X�`���ɂ܂Ƃ߂�
	void packBitmapsToAtlas(int atlasWidth,int atlasHeight);

	//utf-8����shif-jis�ɕϊ�
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

	//�t�H���g�S�̂̍������擾
	const float getFontHeight();

	std::shared_ptr<ImageData> getTexture() { return atlasTexture; }
	VkDescriptorSet& getDescriptorSet() { return atlasTexDescriptorSet; }

	//��̕����̃t�H���g�摜���擾
	void getCharFont(const std::vector<uint32_t>& utf8Codes, std::vector<CharFont>& charFonts);
};