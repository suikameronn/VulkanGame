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

	//文字列からスペースを除いた文字列の長さを数える
	textLengthNonSpace = 0;
	const char* begin = text.c_str();
	const char* end = begin + strlen(text.c_str());

	const std::u8string space = u8" ";

	while (begin < end)
	{
		uint32_t code = utf8::next(begin, end);

		if (!isSpaceCharacter(code))
		{
			//その文字が空白ではない場合
			textLengthNonSpace++;

			//utf8のコードを記録する
			utf8Codes.push_back(code);
		}
	}

	//スペースを除いた文字数の数だけ、ポリゴンを作成する
	vertices.resize(UIVertexCount * textLengthNonSpace);
	mappedBuffers.resize(textLengthNonSpace);
	descriptorSets.resize(textLengthNonSpace);

	indices = { 0, 1, 2, // 最初の三角形
				2, 3, 1 };  // 2番目の三角形
}

//utf8の文字が空白かそうでないか
//空白の場合true
bool Text::isSpaceCharacter(const char32_t code)
{
	return code == U'\u0020' || code == U'\u00A0'
		|| code == U'\u3000';
}