#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include"sol/sol.hpp"

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

	sol::state state;
	sol::load_result script;
	sol::usertype<Object> luaObject;
	sol::usertype<Model> luaModel;

	void initLuaScript(std::string path);
	void setUsertype();
	void convertObjectType();
	void pushCFunctions();

public:

	void testGlue() { std::cout << "glueSuccess" << std::endl; }
	void glueAddObject(Object* obj);


	Scene();
	~Scene();

	std::shared_ptr<Camera> camera;

	void init(std::string luaScriptPath);


	//ここにシーンのオブジェクトの名前とモデルが保存される
	std::vector<std::shared_ptr<Model>> sceneSet;

	bool UpdateScene();
};

static OBJECT convertEnum(int obj)
{
	switch (obj)
	{
	case 101:
		return OBJECT::gltfTEST;
	case 102:
		return OBJECT::CUBE;
	}
}

/*

static int glueTestFunc(lua_State* pL)
{
	std::cout << "LUA" << std::endl;

	return 0;
}

static int glueSetUpdateScript(lua_State* pL)
{
	std::string name;
	std::string luaScriptPath;
	luaScriptPath = "C:/Users/sukai/Documents/VulkanGame/LusScripts/";

	name = lua_tostring(pL, -2);
	luaScriptPath += lua_tostring(pL, -1);

	Scene::GetInstance()->sceneSet[name]->setLuaScript(luaScriptPath);

	return 0;
}



static int glueAddPlayer(lua_State* pL)
{
	std::string name;
	int objNumber;

	name = lua_tostring(pL, -2);
	objNumber = lua_tonumber(pL, -1);

	OBJECT obj = convertEnum(objNumber);

	FileManager* fileManager = FileManager::GetInstance();

	Scene::GetInstance()->sceneSet[name] = std::shared_ptr<Player>(new Player());
	Scene::GetInstance()->sceneSet[name]->setgltfModel(fileManager->loadModel(obj));

	lua_pop(pL, -1);

	return 0;
}

static int glueAddStageBox(lua_State* pL)
{
	std::string name;
	int objNumber;

	name = lua_tostring(pL, -2);
	objNumber = lua_tonumber(pL, -1);

	OBJECT obj = convertEnum(objNumber);

	FileManager* fileManager = FileManager::GetInstance();

	Scene::GetInstance()->sceneSet[name] = std::shared_ptr<StageBox>(new StageBox());
	Scene::GetInstance()->sceneSet[name]->setgltfModel(fileManager->loadModel(obj));

	lua_pop(pL, -1);

	return 0;
}

static int glueSetPos(lua_State* pL)
{
	std::string name;
	float x;
	float y;
	float z;

	name = lua_tostring(pL, -4);
	x = lua_tonumber(pL, -3);
	y = lua_tonumber(pL, -2);
	z = lua_tonumber(pL, -1);

	Scene::GetInstance()->sceneSet[name]->setPosition(glm::vec3(x, y, z));

	lua_pop(pL, -1);

	return 0;
}

static int glueSetRotate(lua_State* pL)
{
	std::string name;
	float x;
	float y;
	float z;

	name = lua_tostring(pL, -4);
	x = lua_tonumber(pL, -3);
	y = lua_tonumber(pL, -2);
	z = lua_tonumber(pL, -1);

	Rotate rotate = { x,y,z };
	Scene::GetInstance()->sceneSet[name]->rotate = rotate;

	lua_pop(pL, -1);

	return 0;
}

int glueSetScale(lua_State* pL)
{
	std::string name;
	float x;
	float y;
	float z;

	name = lua_tostring(pL, -4);
	x = lua_tonumber(pL, -3);
	y = lua_tonumber(pL, -2);
	z = lua_tonumber(pL, -1);

	Scene::GetInstance()->sceneSet[name]->scale = glm::vec3(x, y, z);

	lua_pop(pL, -1);

	return 0;
}

static int glueSetColider(lua_State* pL)
{
	std::string name;

	name = lua_tostring(pL, -1);

	Scene::GetInstance()->sceneSet[name]->setColider();

	lua_pop(pL, -1);

	return 0;
}

static int glueBindCamera(lua_State* pL)
{
	std::string name;

	name = lua_tostring(pL, -1);

	Scene* scene = Scene::GetInstance();

	scene->sceneSet[name]->bindCamera(std::weak_ptr<Object>(scene->camera));

	lua_pop(pL, -1);

	return 0;
}

static int glueBindObject(lua_State* pL)
{
	std::string parentName;
	std::string childName;

	Scene* scene = Scene::GetInstance();

	scene->sceneSet[parentName]->bindObject(std::weak_ptr<Object>(scene->sceneSet[childName]));

	lua_pop(pL, -1);

	return 0;
}

*/