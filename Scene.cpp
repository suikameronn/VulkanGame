#include"Scene.h"

Scene::Scene()
{
}

void Scene::init(std::string luaScriptPath)
{

	camera = std::make_shared<Camera>();

	initLuaScript(luaScriptPath);

	parseScene();

	setModels();
}

Scene::~Scene()
{
}

void Scene::initLuaScript(std::string path)
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);
	lua_setglobal(lua, "Scene");

	registerOBJECT();

	createModelMetatable();

	lua_register(lua, "glueCreateModel", glueCreateModel);

	luaL_dofile(lua,path.c_str());
}

void Scene::registerOBJECT()
{
	lua_pushnumber(lua, (int)OBJECT::gltfTEST);
	lua_setglobal(lua, "gltfTEST");

	lua_pushnumber(lua, (int)OBJECT::CUBE);
	lua_setglobal(lua, "CUBE");
}

void Scene::createModelMetatable()
{
	luaL_newmetatable(lua, "ModelMetaTable");



	lua_pushcfunction(lua, glueAllGc);
	lua_setfield(lua, -2, "__gc");
	lua_pushcfunction(lua, glueSetGltfModel);
	lua_setfield(lua, -2, "setGltfModel");

	lua_pop(lua, 1);
}

void Scene::parseScene()
{
}

bool Scene::UpdateScene()
{
	bool exit = false;

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		(*itr)->Update();
	}

	camera->Update();

	for (auto itr = sceneSet.begin(); itr != std::prev(sceneSet.end()); itr++)
	{
		if (!(*itr)->hasColider())
		{
			continue;
		}

		for (auto itr2 = std::next(itr); itr2 != sceneSet.end(); itr2++)
		{
			if ((*itr2)->hasColider())
			{
				if ((*itr)->getColider()->Intersect((*itr2)->getColider(), collisionDepth, collisionVector))
				{
					(*itr)->setPosition((*itr)->getPosition() + collisionVector * collisionDepth);
				}
			}
		}
	}

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if ((*itr)->uniformBufferChange || true)
		{
			(*itr)->updateTransformMatrix();
		}
	}

	return exit;
}

void Scene::setModels()
{
	Storage::GetInstance()->setCamera(camera);

	//描画するモデルのポインタを積んでいく
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		(*itr)->updateTransformMatrix();
		Storage::GetInstance()->addModel(*itr);
	}
}