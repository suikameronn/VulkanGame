#pragma once

#include<iostream>
#include<array>
#include <numeric> 

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include<vulkan/vulkan.h>

#include"Model.h"

#define PI 3.14159265359

#define UIVertexCount 4
#define UIIndexCount 6

//UIの表示優先度を示す 降順
enum class UILayer
{
	MENU = 0,//最前列
	INGAME,
	BACKGROUND
};

//2D用の頂点構造体
struct Vertex2D
{
	//z値は、UIをレンダイングする際の優先順位に使用
	glm::vec3 pos;
	glm::vec2 uv;
};

//回転の構造体
//各軸に度数法単位の回転角度を指定する
struct Rotate2D
{
	float z;

	//ラジアンへ変換
	float getRadian(float deg)
	{
		return static_cast<float>(deg * (PI / 180.0f));
	}

	//回転行列の取得
	glm::mat4 getRotateMatrix()
	{
		glm::mat4 mat = glm::rotate(glm::mat4(1.0f), getRadian(z),glm::vec3(0.0f,0.0f,1.0f));

		return mat;
	}
};

//UI用の2D行列
struct MatricesUBO2D
{
	glm::mat4 transformMatrix;
	glm::mat4 projection;
};

class UI : public std::enable_shared_from_this<UI>
{
protected:

	bool initFrame;

	//UIを表示するレイヤー
	UILayer layer;

	//長方形のインデックス
	std::vector<uint32_t> indices;

	//長方形の頂点
	std::vector<Vertex2D> vertices;

	//ui画像のアスペクト比
	glm::vec2 aspect;

	//uiの幅と高さ
	float uiWidth, uiHeight;

	//座標変換行列
	glm::mat4 transformMatrix;

	//UIのタイプ
	UINum uiNum;

	//uiの伸縮
	float scale;

	//UIの存在フラッグ
	bool exist;

	//半透明描画のフラッグ
	bool transparent;

	//UI用のプロジェクション行列
	glm::mat4 projMatrix;

	//座標変換行列の更新フラッグ
	bool isUpdateTransformMatrix;

	//画面上のuiの座標
	glm::vec2 position;

	//uiの回転の情報
	Rotate2D rotate;

	//uiの表示非表示
	bool isVisible;

	//uiのテクスチャを張り付けるための長方形のバッファ
	std::array<BufferObject, 2> pointBuffers;

	//UI用の画像が結び付けられている
	VkDescriptorSet transformDescriptorSet;
	VkDescriptorSet imageDescriptorSet;

	//uniform buffer用のバッファ
	MappedBuffer mappedBuffer;

	//画像データ(uiイラストなど)
	std::shared_ptr<ImageData> uiImage;

	void updateUniformBuffer();

	virtual void cleanupVulkan();

public:

	UI() {};
	UI(std::shared_ptr<ImageData> image);
	~UI()
	{
		//gpu上のバッファなどを破棄
		cleanupVulkan();
	}

	UINum getUINum()
	{
		return uiNum;
	}

	bool isInitFrame()
	{
		return initFrame;
	}

	bool isExist()
	{
		return exist;
	}

	bool isTransparent()
	{
		return transparent;
	}

	void setTransparent(bool t)
	{
		transparent = t;
	}

	//座標の設定
	void setPosition(glm::vec2 pos);
	//回転の設定
	void setRotate(Rotate2D rot);
	//伸縮の設定
	void setScale(float scale);

	//テクスチャの幅と高さを取得
	float getTexWidth() { return uiWidth; }
	float getTexHeight() { return uiHeight; }

	//表示非表示の設定 引数によって設定
	void setVisible(bool visible);
	bool getVisible() { return isVisible; }

	uint32_t getVerticesSize() { return static_cast<uint32_t>(vertices.size()); }
	uint32_t getIndicesSize() { return static_cast<uint32_t>(indices.size()); }

	//頂点配列の取得
	Vertex2D* getVertices();

	//インデックス配列の取得
	uint32_t* getIndices();

	//gpu上のバッファの数
	const int getPointBufferSize() { return static_cast<int>(pointBuffers.size()); }

	//gpu上の頂点に関するバッファを取得
	BufferObject* getPointBuffer();

	//uniform bufferを取得
	MappedBuffer& getMappedBuffer() { return mappedBuffer; }

	//descriptroSetの取得
	VkDescriptorSet& getTransformDescriptorSet() { return transformDescriptorSet; }
	virtual VkDescriptorSet& getImageDescriptorSet() { return imageDescriptorSet; }

	//uiとして見られるテクスチャの取得
	TextureData* getUITexture();

	//プロジェクション行列を返す
	glm::mat4 getProjectMatrix() { return projMatrix; }

	//ui画像を返す
	std::shared_ptr<ImageData> getImageData();

	//テクスチャ画像の設定とテクスチャの作成
	void createTexture(std::shared_ptr<ImageData> image);

	//座標変換行列の更新
	virtual void updateTransformMatrix();

	//初期フレームの時に実行する
	virtual void initFrameSettings();

	//フレーム終了時に実行する
	void frameEnd();

	//座標変換行列を返す
	glm::mat4 getTransfromMatrix() { return transformMatrix; }
};