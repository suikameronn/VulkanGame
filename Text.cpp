#include"GameManager.h"

#include"Text.h"

Text::Text(std::string text)
{
	uiNum = UINum::TEXT;

	exist = true;

	GameManager* manager = GameManager::GetInstance();

	projMatrix = manager->getUIProjection();

	isUpdateTransformMatrix = true;

	position = glm::vec2(0.0f, 0.0f);

	rotate.z = 0.0f;

	scale = 1.0f;

	isVisible = true;
	transparent = true;

	//文字列からスペースを除いた文字列の長さを数える
	textLengthNonSpace = 0;
	const char* begin = text.c_str();
	const char* end = begin + strlen(text.c_str());

	//改行の位置を示す
	std::vector<int> startNewLinePoint;

	int index = 0;
	while (begin < end)
	{
		uint32_t code = utf8::next(begin, end);

		if (isStartNewLine(code))
		{
			//改行の位置を記録する
			startNewLinePoint.push_back(index);
			index++;
			continue;
		}

		if (!isSpaceCharacter(code))
		{
			//その文字が空白ではない場合
			textLengthNonSpace++;
		}

		//utf8のコードを記録する
		utf8Codes.push_back(code);

		index++;
	}

	//文字列から必要なフォントのデータを取得する
	loadFont();

	//文字列から頂点データを設定する
	setVertIndices(startNewLinePoint);

	//UIを表示するための行列を記録するためのバッファを作成
	VulkanBase::GetInstance()->uiCreateUniformBuffer(mappedBuffer);

	setPosition(glm::vec2(manager->getWindowWidth() / 2.0f, manager->getWindowHeight() / 2.0f));
}

//utf8の文字が空白かそうでないか
//空白の場合true
bool Text::isSpaceCharacter(const char32_t code)
{
	return code == U'\u0020' || code == U'\u00A0'
		|| code == U'\u3000';
}

//改行の記号かどうか
bool Text::isStartNewLine(const char32_t code)
{
	return code == U'\u000D' || code == U'\u000A';
}

//文字列から必要なフォントのデータを取得する
void Text::loadFont()
{
	FontManager* fontManager = FontManager::GetInstance();

	fontManager->getCharFont(utf8Codes, charFonts);

	//得たフォントデータから文字列をレンダリングしたときの大きさを取得する
	uiWidth = 0;
	uiHeight = fontManager->getFontHeight();
	for (const CharFont& charFont : charFonts)
	{
		uiWidth += charFont.advance;
	}
}

//文字列から頂点データを設定する
void Text::setVertIndices(std::vector<int>& startNewLinePoint)
{
	//スペースを除いた文字数の数だけ、ポリゴンを作成する
	vertices.resize(textLengthNonSpace * UIVertexCount);
	indices.resize(textLengthNonSpace * UIIndexCount);

	//indices = { 0, 1, 2, // 最初の三角形
	//			2, 3, 1 };  // 2番目の三角形

	//文字一つ一つのポリゴンに上のインデックスを
	//文字数に合わせて調整したものを設定する
	for (int i = 0; i < textLengthNonSpace; i++)
	{
		indices[i * UIIndexCount] = 0 + (i * UIVertexCount);
		indices[i * UIIndexCount + 1] = 1 + (i * UIVertexCount);
		indices[i * UIIndexCount + 2] = 2 + (i * UIVertexCount);
		indices[i * UIIndexCount + 3] = 2 + (i * UIVertexCount);
		indices[i * UIIndexCount + 4] = 3 + (i * UIVertexCount);
		indices[i * UIIndexCount + 5] = 1 + (i * UIVertexCount);
	}

	//フォントから頂点の座標を設定する
	glm::vec2 penPos = glm::vec2(0.0f);
	for (int i = 0; i < charFonts.size(); i++)
	{
		if (charFonts[i].size.x == 0.0f)
		{
			//もしスペースの場合はペンの位置だけずらす
			penPos.x += charFonts[i].advance;
			continue;
		}

		for (int index : startNewLinePoint)
		{
			if (i == index)
			{
				penPos.x = 0;
				penPos.y += charFonts[i].fontHeight;
			}
		}

		//頂点座標の設定
		vertices[i * UIVertexCount + 0].pos = glm::vec2(charFonts[i].bearing.x, charFonts[i].bearing.y) + penPos;

		vertices[i * UIVertexCount + 1].pos = glm::vec2(charFonts[i].bearing.x + charFonts[i].size.x
			, charFonts[i].bearing.y) + penPos;

		vertices[i * UIVertexCount + 2].pos = glm::vec2(charFonts[i].bearing.x
			, charFonts[i].bearing.y + charFonts[i].size.y) + penPos;

		vertices[i * UIVertexCount + 3].pos = glm::vec2(charFonts[i].bearing.x + charFonts[i].size.x
			, charFonts[i].bearing.y + charFonts[i].size.y) + penPos;

		penPos.x += charFonts[i].advance;

		//uv座標の設定
		if (charFonts[i].rect.rotated)
		{
			//ビットマップが回転させられている場合
			//uvをずらす

			vertices[i * UIVertexCount + 0].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMax.y);

			vertices[i * UIVertexCount + 1].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMin.y);

			vertices[i * UIVertexCount + 2].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMax.y);

			vertices[i * UIVertexCount + 3].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMin.y);
		}
		else
		{
			//回転していない場合は、そのままuvを設定する
			vertices[i * UIVertexCount + 0].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMin.y);

			vertices[i * UIVertexCount + 1].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMin.y);

			vertices[i * UIVertexCount + 2].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMax.y);

			vertices[i * UIVertexCount + 3].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMax.y);
		}
	}
}

//座標変換行列を設定
void Text::initFrameSettings()
{
	updateTransformMatrix();
}

//gpu上のバッファを破棄
void Text::cleanupVulkan()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	for (BufferObject& pointBuffer : pointBuffers)
	{
		vulkan->addDefferedDestructBuffer(pointBuffer);
	}
	vulkan->addDefferedDestructBuffer(mappedBuffer);
}