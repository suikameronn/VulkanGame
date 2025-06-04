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

	//文字のフォントを張り付けるポリゴンの頂点
	//ポリゴン一つに文字を張り付ける
	std::vector<Vertex2D> vertices;
	std::vector<BufferObject> pointBuffers;

	//文字ごとにポリゴンを移動させる行列を用意する
	std::vector<glm::mat4> transformMatrices;
	//文字ごとにポリゴンを移動させる行列を用意する
	std::vector<MappedBuffer> mappedBuffers;
	//文字ごとのポリゴン用のVkDescriptorSet
	std::vector<VkDescriptorSet> descriptorSets;

	//utf8の文字が空白かそうでないか
	//空白の場合true
	bool isSpaceCharacter(const char32_t code);

public:

	Text(std::string str);

	//頂点配列を返す
	const std::vector<Vertex2D>& getVertices() { return vertices; }
	//頂点用のバッファの配列を返す
	std::vector<BufferObject> getPointBuffer() { return pointBuffers; }
	//行列記録用のバッファの配列を返す
	std::vector<MappedBuffer>& getMappedBuffer() { return mappedBuffers; }
	//VkDescriptorSetを返す
	std::vector<VkDescriptorSet>& getDescriptorSet() { return descriptorSets; }

	//テキストを描画する際のポリゴンのインデックスが記録されている
	//BufferObjectの配列の添え字を返す
	const int getIndexBufferNum() { return indexBufferNum; }
};