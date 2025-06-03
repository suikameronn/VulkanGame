#pragma once

#include"UI.h"
#include"FontManager.h"

#include"utf8/checked.h"

class Text : public UI
{
private:

	//表示する文字
	std::string text;
	//スペースを除いた文字数
	int textLengthNonSpace;

	//文字のフォントを張り付けるポリゴンの頂点
	//ポリゴン一つに文字を張り付ける
	std::vector<Vertex2D> vertices;

	//フォントごとにポリゴンを移動させる行列を用意する
	std::vector<glm::mat4> transformMatrices;
	//フォントごとにポリゴンを移動させる行列を用意する
	std::vector<MappedBuffer> mappedBuffers;

public:

	Text(std::string str);
};