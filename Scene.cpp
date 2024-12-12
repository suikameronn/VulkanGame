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

	setUsertype();
	this->state.set_function("glueAddObject", &Scene::glueAddObject, this);

	script = state.load_file(path);
	if (!script.valid())
	{
		std::cout << "lua load error" << std::endl;
	}

}

void Scene::setUsertype()
{
	luaObject = state.new_usertype<Object>("Object", sol::constructors<Object()>());
	luaModel = state.new_usertype<Model>("Model", sol::constructors<Model()>());
}

void Scene::parseScene()
{

	script();
}

void Scene::glueAddObject(Object* obj)
{
	switch (obj->getObjNum())
	{
	case cObject:
		std::cout << 0 << std::endl;
		break;
	case cModel:
		std::cout << 1 << std::endl;
		break;
	default:
		std::cout << "none case" << std::endl;
	}
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