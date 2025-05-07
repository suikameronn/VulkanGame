#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include "lua/include/lua.hpp"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"

#include"FileManager.h"
#include"Controller.h"
#include"Player.h"
#include"Camera.h"
#include"EnumList.h"
#include"Light.h"
#include"UI.h"
#include"Cubemap.h"

#include<chrono>
#include <thread>

enum
{
	GAME_CONTINUE = 0,
	GAME_FINISH,
	GAME_RESTART
};

//ポイントライトの構造体 複数のライトを持つ
struct PointLightUBO
{
	alignas(16)int lightCount;
	alignas(16) std::array<glm::vec4, 50> pos;
	alignas(16) std::array<glm::vec4, 50> color;
};

//平行光源の構造体 複数のライトを持つ
struct DirectionalLightUBO
{
	alignas(16) int lightCount;
	alignas(16) std::array<glm::vec4, 50> dir;
	alignas(16) std::array<glm::vec4, 50> color;
};

//シャドウマップ作成用のuniform buffer
struct ShadowMapUBO
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct DirectionalLightBuffer
{
	DirectionalLightUBO ubo;
	MappedBuffer mappedBuffer;
	VkDescriptorSet descriptorSet;
};

struct PointLightBuffer
{
	PointLightUBO ubo;
	MappedBuffer mappedBuffer;
	VkDescriptorSet descriptorSet;
};

//シャドウマップ作成用の構造体
struct ShadowMapData
{
	//平衡投影の行列
	glm::mat4 proj;

	//シャドウマップの解像度の倍率を設定
	int shadowMapScale;
	//オフスクリーンレンダリング用の構造体
	OffScreenPass passData;
	//シャドウマップ作成用の行列の配列
	std::vector<ShadowMapUBO> matUBOs;
	//行列用のバッファの配列
	std::vector<MappedBuffer> mappedBuffers;

	//シャドウマップを通常のレンダリングで使用するためのデータ
	std::vector<VkDescriptorSet> descriptorSets;

	//シーン上のライトの数だけ作成
	void setFrameCount(int frameCount)
	{
		matUBOs.resize(frameCount);
		mappedBuffers.resize(frameCount);
		passData.setFrameCount(frameCount);
	}

	void destroy(VkDevice& device)
	{
		passData.destroy(device);

		for (auto& buffer : mappedBuffers)
		{
			buffer.destroy(device);
		}
	}
};


//luaから読み取ったステージの情報などが格納される
class Scene
{
private:

	//シーン全体のポイントライト用のバッファ
	PointLightBuffer pointLightBuffer;
	//シーン全体のディレクショナルライト用のバッファ
	DirectionalLightBuffer dirLightBuffer;
	//シャドウマップ用バッファ
	ShadowMapData shadowMapData;

	static Scene* instance;

	std::unique_ptr<RTree<Model>> rtree;

	//上方向のベクトル
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 startPoint;//初期座標
	float limitY;//y座標の下限 これを下回るとリスタート判定

	//初回フレームのみ実行 ステージ上のすべてのオブジェクトの初回フレーム時の設定を行う
	void initFrameSetting();

	//フレーム終了時に実行
	void frameEnd();

	float collisionDepth;//衝突時のめり込んだ距離
	glm::vec3 collisionVector;//衝突時のめり込んだ方向

	//luaの仮想マシンにアクセスするための変数
	lua_State* lua;
	//luaスクリプトの読み取り、実行
	void initLuaScript(std::string path);
	//luaの仮想マシンにgltfモデルを指定するための番号を設定する
	void registerOBJECT();
	//luaから呼び出される静的関数を設定
	void registerFunctions();

	//コライダーが床に接地しているかの判定 trueの場合はその床の移動を追従する
	bool groundCollision(glm::vec3 collisionVector);
	//オブジェクトの接地判定などをリセット
	void resetStatus();

	//R木を使った当たり判定
	void rtreeIntersect();
	//シンプルな当たり判定
	void intersect();

	//ライトのユニフォームバッファの更新
	void updateLightUniformBuffer();
	//ディレクショナルライトの更新
	void updateDirLightUniformBuffer();
	//ポイントライトの更新
	void updatePointLightUniformBuffer();
	//シャドウマップの更新
	void updateShadowMapUniformBuffer();

	Scene();
	~Scene();

	//レンダリング
	void render();

	//gpu上のバッファを破棄する
	//シーン管轄のインスタンスのバッファのみ破棄する
	void cleanupVulkan();

public:

	static Scene* GetInstance()
	{
		if (!instance)
		{
			instance = new Scene();
		}

		return instance;
	}

	void Destroy()
	{
		delete instance;
		instance = nullptr;
	}

	//カメラ
	std::shared_ptr<Camera> camera;

	//luaスクリプトを実行し、登録されたオブジェクトなどの初期フレーム処理まで行う
	void init(std::string luaScriptPath);

	void setStartPoint(glm::vec3 point);//初期座標の設定
	void setLimitY(float y);//y座標の下限の設定
	
	//プレイヤー
	std::shared_ptr<Player> player;
	//キューブマッピング用の立方体オブジェクト
	std::shared_ptr<Cubemap> cubemap;
	//ステージ上のオブジェクトの配列
	std::vector<std::shared_ptr<Model>> sceneModels;
	//ステージ上のポイントライトの配列
	std::vector<std::shared_ptr<PointLight>> scenePointLights;
	//ステージ上の平行光源の配列
	std::vector<std::shared_ptr<DirectionalLight>> sceneDirectionalLights;
	//ステージ上のUI
	std::vector<std::shared_ptr<UI>> sceneUI;

	//ステージ上のオブジェクトなどの更新処理
	int UpdateScene();

	//四角形を延ばして、衝突判定を行う、接地判定に使われる
	std::shared_ptr<Model> raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model,glm::vec3& normal);

	//HDRIマップの設定
	void setHDRIMap(std::string imagePath);

	//シーン全体のオブジェクトについてのRTreeにオブジェクトを追加する
	void addModelToRTree(Model* model);

	//Rツリー内のオブジェクトの位置を更新する
	void updateObjectPos(Model* model, RNode<Model>* node);
};

/*以下の関数はluaスクリプトから呼び出される*/

// スタックの内容を表示する関数 デバッグ用
static void printStack(lua_State* L) {
	int top = lua_gettop(L); // スタックのトップインデックスを取得
	
	std::cout << "Stack size: " << top << std::endl;
	for (int i = 1; i <= top; ++i) {
		int t = lua_type(L, i);
		switch (t) {
		case LUA_TSTRING: // 文字列の場合
			std::cout << i << ": " << lua_tostring(L, i) << " (string)" << std::endl;
			break;
		case LUA_TBOOLEAN: // 真偽値の場合
			std::cout << i << ": " << (lua_toboolean(L, i) ? "true" : "false") << " (boolean)" << std::endl;
			break;
		case LUA_TNUMBER: // 数値の場合
			std::cout << i << ": " << lua_tonumber(L, i) << " (number)" << std::endl;
			break;
		case LUA_TUSERDATA: // ユーザーデータの場合
			std::cout << i << ": " << lua_touserdata(L, i) << " (userdata)" << std::endl;
			break;
		case LUA_TTABLE: // テーブルの場合
			std::cout << i << ": table" << std::endl;
			break;
		default: // その他の場合
			std::cout << i << ": " << lua_typename(L, t) << std::endl;
			break;
		}
	}
}

namespace glueSceneFunction//Sceneクラスの用のglue関数
{

	//オブジェクトを作成する 通常は使わない
	static int glueCreateObject(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		std::shared_ptr<Object> obj = std::shared_ptr<Object>(new Object());

		return 1;
	}

	//3Dモデルを持つオブジェクトを作成する
	static int glueCreateModel(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		Model* model = new Model();

		lua_pushlightuserdata(lua, model);

		std::shared_ptr<Model> sharedModel = std::shared_ptr<Model>(model);
		scene->sceneModels.push_back(sharedModel);

		return 1;
	}

	//ポイントライトの作成
	static int glueCreatePointLight(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		PointLight* pointLight = new PointLight();
		scene->scenePointLights.push_back(std::shared_ptr<PointLight>(pointLight));

		lua_pushlightuserdata(lua, pointLight);

		return 1;
	}

	//平行光源の作成
	static int glueCreateDirectionalLight(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		DirectionalLight* directionalLight = new DirectionalLight();
		scene->sceneDirectionalLights.push_back(std::shared_ptr<DirectionalLight>(directionalLight));

		lua_pushlightuserdata(lua, directionalLight);

		return 1;
	}

	//プレイヤーの作成 すでに作成されていた場合は何もしない
	static int glueCreatePlayer(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		if (scene->player)
		{
			return -1;
		}

		Player* player = new Player();
		scene->player = std::shared_ptr<Player>(player);

		lua_pushlightuserdata(lua, player);

		return 1;
	}

	//オブジェクトに行動パターン用のluaスクリプトを設定する
	static int glueSetLuaPath(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		std::string path = std::string(lua_tostring(lua, -1));

		obj->setLuaScript(path);

		return 0;
	}

	//luaスクリプトを実行し始めるフレームを遅延させる
	static int glueSetDelayStartLua(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		int delayFrame = static_cast<int>(lua_tointeger(lua, -1));

		obj->setDelayFrameCount(delayFrame);

		return 0;
	}

	//引数としてModelクラスのインスタンスとgltfモデルの種類を示す番号を受け取り
	//Modelクラスにgltfモデルを設定する
	static int glueSetGltfModel(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		std::string filePath = lua_tostring(lua, -1);

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			FileManager::GetInstance()->addLoadModelList(filePath, model);
			break;
		}


		return 0;
	}

	//オブジェクトの座標を設定する
	static int glueSetPos(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		{
			float x = static_cast<float>(lua_tonumber(lua, -3));
			float y = static_cast<float>(lua_tonumber(lua, -2));
			float z = static_cast<float>(lua_tonumber(lua, -1));
			obj->setPosition(glm::vec3(x, y, z));
			break;
		}
		}

		return 0;
	}

	//回転を設定する 単位は度数法
	static int glueSetRotate(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
			obj->rotate.x = static_cast<float>(lua_tonumber(lua, -3));
			obj->rotate.y = static_cast<float>(lua_tonumber(lua, -2));
			obj->rotate.z = static_cast<float>(lua_tonumber(lua, -1));
			break;
		}

		return 0;
	}

	//スケールを設定する
	static int glueSetScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);

			glm::vec3 scale;
			for (int i = 0; i < 3; i++)
			{
				scale[i] = static_cast<float>(lua_tonumber(lua, -3 + i));
			}

			model->scale = scale;
			break;
		}

		return 0;
	}


	//gltfモデルのベースカラーを上書きする
	static int glueSetBaseColor(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -5));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			float r = static_cast<float>(lua_tonumber(lua, -4));
			float g = static_cast<float>(lua_tonumber(lua, -3));
			float b = static_cast<float>(lua_tonumber(lua, -2));
			float a = static_cast<float>(lua_tonumber(lua, -1));
			glm::vec4 baseColor = glm::vec4(r, g, b, a);
			model->setBaseColor(baseColor);
		}

		return 0;
	}

	//カメラが追従するオブジェクトを設定する
	static int glueBindCamera(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		obj->bindCamera(std::weak_ptr<Camera>(scene->camera));

		return 0;
	}

	//オブジェクトにAABBコライダーを設定する
	static int glueSetAABBColider(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->setColider();
			model->isMovable = static_cast<bool>(lua_toboolean(lua, -1));
			break;
		}

		return 0;
	}

	//コライダーのスケールを設定する
	static int glueSetColiderScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			std::shared_ptr<Colider> colider = model->getColider();
			if (colider)
			{
				float x = static_cast<float>(lua_tonumber(lua, -3));
				float y = static_cast<float>(lua_tonumber(lua, -2));
				float z = static_cast<float>(lua_tonumber(lua, -1));
				colider->scale = glm::vec3(x, y, z);
			}
			break;
		}

		return 0;
	}

	//オブジェクトのアイドル時に再生するアニメーションの名前を設定する
	static int glueSetDefaultAnimationName(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->setDefaultAnimationName(std::string(lua_tostring(lua, -1)));
			break;
		}

		return 0;
	}

	//オブジェクトが従う重力の強さを設定する
	static int glueSetGravity(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->gravity = static_cast<float>(lua_tonumber(lua, -1));
			break;
		}

		return 0;
	}

	//ライトの光の色を設定する
	static int glueSetLightColor(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));
		glm::vec3 color =
		{
			static_cast<float>(lua_tonumber(lua, -3)),
			static_cast<float>(lua_tonumber(lua, -2)),
			static_cast<float>(lua_tonumber(lua, -1))
		};

		switch (obj->getObjNum())
		{
		case 3://ポイントライト用
		{
			PointLight* pl = dynamic_cast<PointLight*>(obj);
			pl->color = color;
			break;
		}
		case 4://平行光源用
		{
			DirectionalLight* dl = dynamic_cast<DirectionalLight*>(obj);
			dl->color = color;
			break;
		}
		}

		return 0;
	}

	//平行光源の光の方向を設定する
	static int glueSetLightDirection(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));
		glm::vec3 dir =
		{
			static_cast<float>(lua_tonumber(lua, -3)),
			static_cast<float>(lua_tonumber(lua, -2)),
			static_cast<float>(lua_tonumber(lua, -1))
		};

		switch (obj->getObjNum())
		{
		case 4://平行光源のみ
		{
			DirectionalLight* dl = dynamic_cast<DirectionalLight*>(obj);
			dl->direction = dir;
			break;
		}
		}

		return 0;
	}

	//引数の一つ目のオブジェクトを親として、二つのオブジェクトに親子関係を設定する
	static int glueBindObject(lua_State* lua)
	{
		Object* parent = static_cast<Object*>(lua_touserdata(lua, -2));
		Object* child = static_cast<Object*>(lua_touserdata(lua, -1));

		parent->bindObject(child);

		return 0;
	}

	//オブジェクトのリスタート時の座標を設定する
	static int glueSetStartPoint(lua_State* lua)
	{
		float x, y, z;
		x = static_cast<float>(lua_tonumber(lua, -3));
		y = static_cast<float>(lua_tonumber(lua, -2));
		z = static_cast<float>(lua_tonumber(lua, -1));
		glm::vec3 startPoint = { x,y,z };

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		scene->setStartPoint(startPoint);

		return 0;
	}

	//ステージの下限の座標を設定する
	static int glueSetLimitY(lua_State* lua)
	{
		float y;
		y = static_cast<float>(lua_tonumber(lua, -1));

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		scene->setLimitY(y);

		return 0;
	}

	//キューブマップとIBLのもとのHDRI画像を設定する
	static int glueSetHDRIMap(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		scene->setHDRIMap(lua_tostring(lua, -2));

		return 0;
	}

	//uvを調整するようにして、テクスチャの引き延ばしを防ぐ
	static int glueSetUVScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->setUVScale();
			break;
		}

		return 0;
	}
};