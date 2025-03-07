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

#include<chrono>
#include <thread>

class Scene
{
private:

	static Scene* instance;

	std::shared_ptr<ImageData> hdriMap;

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 startPoint;//初期座標
	float limitY;//y座標の下限

	void initFrameSetting();
	void setModels();
	void setLights();

	float upBlend;
	float collisionDepth;//衝突時のめり込んだ距離
	glm::vec3 collisionVector;//衝突時のめり込んだ方向

	lua_State* lua;
	void initLuaScript(std::string path);
	void registerOBJECT();
	void registerFunctions();

	bool groundCollision(glm::vec3 collisionVector);
	void resetStatus();

	Scene();
	~Scene();

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

	std::shared_ptr<Camera> camera;

	void init(std::string luaScriptPath);

	void setStartPoint(glm::vec3 point);//初期座標の設定
	void setLimitY(float y);//y座標の下限の設定
	
	std::shared_ptr<Player> player;
	std::shared_ptr<Model> cubemap;
	std::vector<std::shared_ptr<Model>> sceneModels;
	std::vector<std::shared_ptr<PointLight>> scenePointLights;
	std::vector<std::shared_ptr<DirectionalLight>> sceneDirectionalLights;

	bool UpdateScene();

	std::shared_ptr<Model> raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model);
};

// スタックの内容を表示する関数
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

	//オブジェクトのインスタンスの作成のグルー
	static int glueCreateObject(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		std::shared_ptr<Object> obj = std::shared_ptr<Object>(new Object());

		return 1;
	}

	static int glueCreateModel(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		Model* model = new Model();
		scene->sceneModels.push_back(std::shared_ptr<Model>(model));

		lua_pushlightuserdata(lua, model);

		return 1;
	}

	static int glueCreatePointLight(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		PointLight* pointLight = new PointLight();
		scene->scenePointLights.push_back(std::shared_ptr<PointLight>(pointLight));

		lua_pushlightuserdata(lua, pointLight);

		return 1;
	}

	static int glueCreateDirectionalLight(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		DirectionalLight* directionalLight = new DirectionalLight();
		scene->sceneDirectionalLights.push_back(std::shared_ptr<DirectionalLight>(directionalLight));

		lua_pushlightuserdata(lua, directionalLight);

		return 1;
	}

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

	static int glueSetLuaPath(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		std::string path = std::string(lua_tostring(lua, -1));

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		obj->setLuaScript(path);

		return 0;
	}

	static int glueSetGltfModel(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			model->setgltfModel(FileManager::GetInstance()->loadModel(static_cast<GLTFOBJECT>(lua_tointeger(lua, -1))));
			break;
		}


		return 0;
	}

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

	static int glueSetScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			model->scale.x = static_cast<float>(lua_tonumber(lua, -3));
			model->scale.y = static_cast<float>(lua_tonumber(lua, -2));
			model->scale.z = static_cast<float>(lua_tonumber(lua, -1));
			break;
		}

		return 0;
	}

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

	static int glueBindCamera(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		obj->bindCamera(std::weak_ptr<Camera>(scene->camera));

		return 0;
	}

	static int glueSetColider(lua_State* lua)
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

	static int glueSetSlippery(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		float slippery = static_cast<float>(lua_tonumber(lua, -1));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->slippery = slippery;
			break;
		}

		return 0;
	}

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
		case 3:
		{
			PointLight* pl = dynamic_cast<PointLight*>(obj);
			pl->color = color;
			break;
		}
		case 4:
		{
			DirectionalLight* dl = dynamic_cast<DirectionalLight*>(obj);
			dl->color = color;
			break;
		}
		}

		return 0;
	}

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
		case 4:
		{
			DirectionalLight* dl = dynamic_cast<DirectionalLight*>(obj);
			dl->direction = dir;
			break;
		}
		}

		return 0;
	}

	static int glueBindObject(lua_State* lua)
	{
		Object* parent = static_cast<Object*>(lua_touserdata(lua, -2));
		Object* child = static_cast<Object*>(lua_touserdata(lua, -1));

		parent->bindObject(child);

		return 0;
	}

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

	static int glueSetLimitY(lua_State* lua)
	{
		float y;
		y = static_cast<float>(lua_tonumber(lua, -1));

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		scene->setLimitY(y);

		return 0;
	}
};