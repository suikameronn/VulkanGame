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
#include"StageBox.h"
#include"Camera.h"
#include"EnumList.h"

class Scene
{
private:

	void parseScene();

	void setModels();

	float collisionDepth;//衝突時のめり込んだ距離
	glm::vec3 collisionVector;//衝突時のめり込んだ方向

	lua_State* lua;
	void initLuaScript(std::string path);
	void registerOBJECT();
	void createModelMetatable();

public:

	void test() { std::cout << "glueSuccess" << std::endl; }

	Scene();
	~Scene();

	std::shared_ptr<Camera> camera;

	void init(std::string luaScriptPath);

	std::vector<std::shared_ptr<Model>> sceneSet;

	bool UpdateScene();
};

static int testGlue(lua_State* lua)
{
	std::cout << "testGlue" << std::endl;

	lua_getglobal(lua, "Scene");

	Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

	scene->test();

	return 1;
}

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

static int glueAllGc(lua_State* lua)
{
	return 0;
}

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
	scene->sceneSet.push_back(std::shared_ptr<Model>(model));

	void** userdata = (void**)lua_newuserdata(lua, sizeof(void*));
	*userdata = model;

	luaL_getmetatable(lua, "ModelMetaTable");
	lua_setmetatable(lua, -2);

	return 1;
}

static int glueSetGltfModel(lua_State* lua)
{
	std::cout << "aaaa" << std::endl;

	printStack(lua);

	/*
	//void** userdata = (void**)luaL_checkudata(lua,-1, "ModelMetaTable");
	Model* model = static_cast<Model*>(*userdata);

	//const OBJECT modelType = (OBJECT)lua_tointeger(lua, -2);

	FileManager::GetInstance()->loadModel(modelType);
	*/

	return 1;
}