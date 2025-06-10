#pragma once

#include"UI.h"
#include"FontManager.h"

#include"utf8/checked.h"

class Text : public UI
{
private:

	//テキストを表示する際のポリゴンのインデックスを記録している
	//BufferObjectの配列の添え字をしめす
	const int indexBufferNum = 0;

	//表示する文字
	std::string text;
	//utf8の文字コード
	std::vector<uint32_t> utf8Codes;
	//スペースを除いた文字数
	int textLengthNonSpace;

	//文字列のフォントのデータ
	std::vector<CharFont> charFonts;

	//utf8の文字が空白かそうでないか
	//空白の場合true
	bool isSpaceCharacter(const char32_t code);
	
	//改行記号かどうか
	bool isStartNewLine(const char32_t code);

	//文字列から必要なフォントを取得する
	void loadFont();

	//文字列から頂点データを設定する
	void setVertIndices(std::vector<int>& startNewLinePoint);

	void cleanupVulkan() override;

public:

	Text(std::string str);

	void initFrameSettings() override;

	VkDescriptorSet& getImageDescriptorSet() override { return FontManager::GetInstance()->getDescriptorSet(); }
};