#include"Scene.h"

Scene* Scene::globalScene = nullptr;

Scene::Scene()
{
}

void Scene::init(std::string luaScriptPath)
{
	globalScene = this;

	camera = std::make_shared<Camera>();

	initLuaScript(luaScriptPath);

	parseScene();

	setModels();
}

Scene::~Scene()
{
	globalScene = nullptr;

	if (false)
	{
	//	lua_close(pL);
	}
}

void Scene::pushCFunctions()
{
	/*
	lua_pushcfunction(pL,glueTestFunc);
	lua_setglobal(pL, "glueTestFunc");

	lua_pushcfunction(pL, glueAddPlayer);
	lua_setglobal(pL, "glueAddPlayer");

	lua_pushcfunction(pL, glueAddStageBox);
	lua_setglobal(pL, "glueAddStageBox");

	lua_pushcfunction(pL, glueSetPos);
	lua_setglobal(pL, "glueSetPos");

	lua_pushcfunction(pL, glueSetColider);
	lua_setglobal(pL, "glueSetColider");

	lua_pushcfunction(pL, glueSetScale);
	lua_setglobal(pL, "glueSetScale");

	lua_pushcfunction(pL, glueSetRotate);
	lua_setglobal(pL, "glueSetRotate");

	lua_pushcfunction(pL, glueBindCamera);
	lua_setglobal(pL, "glueBindCamera");

	lua_pushcfunction(pL, glueBindObject);
	lua_setglobal(pL, "glueBindObject");

	lua_pushcfunction(pL, glueSetUpdateScript);
	lua_setglobal(pL, "glueSetUpdateScript");
	*/
}

void Scene::initLuaScript(std::string path)
{
	/*
	pL = luaL_newstate();
	luaL_openlibs(pL);
	luaL_loadfile(pL, path.c_str());

	pushCFunctions();

	if (lua_pcall(pL, 0, 0, 0, 0))
	{
		std::cout << "error" << std::endl;
	}
	*/

	state.open_libraries(sol::lib::base, sol::lib::package);

	script = state.load_file(path);
}

void Scene::parseScene()
{
	sol::usertype<Scene> usertype = state.new_usertype<Scene>("Scene", sol::constructors<Scene()>());

	this->state.set_function("testGlue", &Scene::testGlue, this);

	script();
}

bool Scene::UpdateScene()
{
	bool exit = false;

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->Update();
	}

	camera->Update();

	for (auto itr = sceneSet.begin(); itr != std::prev(sceneSet.end()); itr++)
	{
		if (!itr->second->hasColider())
		{
			continue;
		}

		for (auto itr2 = std::next(itr); itr2 != sceneSet.end(); itr2++)
		{
			if (itr2->second->hasColider())
			{
				if (itr->second->getColider()->Intersect(itr2->second->getColider(), collisionDepth, collisionVector))
				{
					itr->second->setPosition(itr->second->getPosition() + collisionVector * collisionDepth);
				}
			}
		}
	}

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if (itr->second->uniformBufferChange || true)
		{
			itr->second->updateTransformMatrix();
		}
	}

	return exit;
}

std::shared_ptr<Model> Scene::getSceneModelData(std::string name)
{
	return sceneSet[name];
}

void Scene::setModels()
{
	Storage::GetInstance()->setCamera(camera);

	//描画するモデルのポインタを積んでいく
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->updateTransformMatrix();
		Storage::GetInstance()->addModel(itr->second);
	}
}

void Scene::setModels(std::string name)
{
	//シーン内の重複したモデルやデータを除いた、シーンに必要なモデルやテクスチャをmodelsPointerに入れる
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if (&itr->second == &sceneSet[name])
		{
			continue;
		}

		Storage::GetInstance()->addModel(itr->second);
	}
}