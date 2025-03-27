#pragma once

#include<iostream>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"VulkanBase.h"

#define PI 3.14159265359

//回転の構造体
//各軸に度数法単位の回転角度を指定する
struct Rotate
{
	float z;

	//ラジアンへ変換
	float getRadian(float deg)
	{
		return static_cast<float>(deg * (PI / 180.0f));
	}

	//回転行列の取得
	glm::mat3 getRotateMatrix()
	{
		glm::mat4 mat = glm::rotate(glm::mat4(1.0f), getRadian(z), glm::vec3(0.0f, 0.0f, 1.0f));

		return glm::mat3(mat);
	}
};

class UI
{
protected:
	//画面上のuiの座標
	glm::vec2 position;

	//uiの回転の情報
	Rotate rotate;

	//uiの伸縮
	glm::vec2 scale;

	//座標変換行列


	//uiの表示非表示
	bool isVisible;

	//長方形の頂点
	std::array<glm::vec2, 4> vertices;

	//uv座標
	std::array<glm::vec2, 4> uv;

	//長方形のインデックス
	std::array<uint32_t, 6> indices;

	//uiのテクスチャを張り付けるための長方形のバッファ
	BufferObject pointBuffer;

	//uiのテクスチャ
	TextureData* uiTexture;

public:

	UI();

	//座標の設定
	void setPosition(glm::vec2 pos);
	//回転の設定
	void setRotate(Rotate rot);
	//伸縮の設定
	void setScale(glm::vec2 scale);

	//表示非表示の設定 引数によって設定
	void setVisible(bool visible);

	//頂点配列の取得
	glm::vec2* getVertices();

	//uv座標の取得
	glm::vec2* getUV();

	//インデックス配列の取得
	uint32_t* getIndices();

	//gpu上の頂点に関するバッファを取得
	BufferObject& getPointBuffer();

	//uiとして見られるテクスチャの取得
	TextureData* getUiTexture();
};