#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define SHOULD_KEEP false
#define SHOULD_DELETE true

#define PI 3.14159265359

#include<iostream>
#include<vector>
#include<math.h>
#include<memory>

#include<vulkan/vulkan.h>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <lua.hpp>
#include <lualib.h>
#include <lauxlib.h>

#include"EnumList.h"
#include"Controller.h"
#include"Camera.h"
#include"PhysicBase.h"

//経過したフレーム数の最大数
#define MAXFRAMECOUNT 10000

//パイプラインレイアウトとパイプライン
struct DescriptorInfo
{
	VkPipelineLayout pLayout;
	VkPipeline pipeline;
};

//uniform bufferを記録するためのバッファー
struct MappedBuffer
{
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;

	void destroy(VkDevice& device)
	{
		vkDestroyBuffer(device, uniformBuffer, nullptr);
		vkFreeMemory(device, uniformBufferMemory, nullptr);
		uniformBufferMapped = nullptr;
	}
};

//レンダリング時のプリミティブとdescriptorSet
struct DescSetData
{
	VkPrimitiveTopology topology;
	VkDescriptorSet descriptorSet;
};

//回転の構造体
//各軸に度数法単位の回転角度を指定する
struct Rotate
{
	float x;
	float y;
	float z;

	//ラジアンへ変換
	float getRadian(float deg)
	{
		return static_cast<float>(deg * (PI / 180.0f));
	}

	//回転行列の取得
	glm::mat4 getRotateMatrix()
	{
		return glm::rotate(glm::mat4(1.0f), getRadian(z), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(y), glm::vec3(0.0f, -1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(x), glm::vec3(1.0f, 0.0f, 0.0f));
	}
};

//すべてのオブジェクトの継承元のクラス、座標の設定、luaスクリプトの設定などを担う
class Object
{
protected:

	//経過したフレーム数
	int passFrameCount;
	//luaスクリプトを実行するのを遅延するフレーム数
	int delayFrameCount;

	//行動パターン用luaスクリプトの仮想マシンにアクセスる変数
	lua_State* lua;
	//スクリプトのコルーチン再開用
	lua_State* coroutine;
	//luaスクリプトのパス
	std::string luaPath;

	//オブジェクトのタイプを示す
	ObjNum objNum;
	//タグを示す
	std::vector<Tag> tags;

	//物理挙動の計算
	std::unique_ptr<PhysicBase> physicBase;

	//カメラを追従させる場合、この変数にカメラへの参照を持たせる
	std::weak_ptr<Camera> cameraObj;
	//自分のオブジェクトに追従する子オブジェクトの配列
	std::vector<Object*> childObjects;

	//回転速度
	float rotateSpeed;

	float length;

	//子オブジェクトに自身の移動などを追従させるために
	//前回のフレームの自身の座標などを記録しておく
	//前回フレームの自分の座標
	glm::vec3 lastPos;
	//回転
	Rotate lastRotate;
	//スケール
	glm::vec3 lastScale;

	//現在の座標
	glm::vec3 position;

	//キー入力の受け取り
	virtual glm::vec3 inputMove();

public:
	Object();

	//自身のオブジェクトのタイプを返す
	ObjNum getObjNum() { return objNum; }
	//指定されたタグを持っているかどうかを返す
	bool containTag(Tag tag);

	//行動パターン用のluaスクリプトを設定する 実行はしない
	void setLuaScript(std::string path);

	//luaスクリプトを実行するのを遅らせるフレーム数を設定する
	void setDelayFrameCount(int delay);

	//親オブジェクトを中心にした球面座標上を動くかどうか
	bool spherePos;

	//移動などにより座標変換行列を更新する必要があるかどうか
	bool uniformBufferChange;

	//回転の情報
	Rotate rotate;

	//親オブジェクトとの距離
	float posOffSet;

	//オブジェクトの正面、右、上のベクトル
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	//自身のモデル行列
	glm::mat4 transformMatrix;

	//引数のオブジェクトを子オブジェクトとして設定する
	void bindObject(Object* obj);
	//カメラを自分に追従させる
	void bindCamera(std::weak_ptr<Camera> camera);
	//自分の移動を子オブジェクトに送り、子オブジェクトを自分の移動に追従させる
	void sendPosToChildren();
	//親オブジェクトの移動を受け取り、親オブジェクトの移動に追従する
	void setParentPos(glm::vec3 lastPos, glm::vec3 currentPos);

	//座標の設定
	virtual void setPosition(glm::vec3 pos);
	//座標の取得
	glm::vec3 getPosition();

	//前回フレームのトランスフォームを取得する
	glm::vec3 getLastPosition();
	Rotate getLastRotate();
	glm::vec3 getLastScale();
	virtual void setLastFrameTransform();//一つ前のフレームの座標などのデータを設定

	//モデル行列を取得
	glm::mat4 getTransformMatrix();

	//luaの仮想マシンに、自身の座標と回転を記録する変数を作成
	virtual void createTransformTable();
	//初回フレームのみ実行、luaスクリプトを実行する
	virtual void initFrameSetting();
	//luaから呼び出される静的関数の登録
	virtual void registerGlueFunctions();
	//モデル行列の更新
	virtual void updateTransformMatrix() {};
	//更新処理
	virtual bool Update();
	//特異な更新処理(通常はなし)
	virtual void customUpdate() {};
	//luaスクリプトに自身の現在の座標、回転の値を送る
	virtual void sendTransformToLua();
	//luaスクリプトから、座標、回転の値を受け取る
	virtual void receiveTransformFromLua();
};

/*以下の関数はluaスクリプトから実行される*/

namespace glueObjectFunction//Objectクラス用のglue関数
{

	//座標の設定
	static int glueSetPos(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		{
			float x = static_cast<float>(lua_tonumber(lua, -4));
			float y = static_cast<float>(lua_tonumber(lua, -3));
			float z = static_cast<float>(lua_tonumber(lua, -2));
			obj->setPosition(glm::vec3(x, y, z));
			break;
		}
		}

		return 0;
	}

	//回転の設定
	static int glueSetRotate(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
			obj->rotate.x = static_cast<float>(lua_tonumber(lua, -4));
			obj->rotate.y = static_cast<float>(lua_tonumber(lua, -3));
			obj->rotate.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}