#include"GameManager.h"

#include"Text.h"

Text::Text(std::string text)
{
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

		if (code != ' ')
		{
			textLengthNonSpace++;
		}
	}

	//スペースを除いた文字数の数だけ、ポリゴンを作成する
	vertices.resize(UIVertexCount * textLengthNonSpace);

	indices = { 0, 1, 2, // 最初の三角形
				2, 3, 1 };  // 2番目の三角形

	//座標変換行列用のバッファの作成
	for (auto& mappedBuffer : mappedBuffers)
	{
		VulkanBase::GetInstance()->uiCreateUniformBuffer(mappedBuffer);
	}
}