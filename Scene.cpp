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

	setEnumOBJECT();
	setUsertype();
	setFunctions();

	script = state.load_file(path);
	if (!script.valid())
	{
		std::cout << "lua load error" << std::endl;
	}

}

void Scene::setEnumOBJECT()
{
	state.new_enum("OBJECT","gltfTEST", OBJECT::gltfTEST, "CUBE", OBJECT::CUBE);
}

void Scene::setUsertype()
{
	luaObject = state.new_usertype<Object>("Object", sol::constructors<Object()>());
	luaModel = state.new_usertype<std::shared_ptr<Model>>("Model", sol::constructors<std::shared_ptr<Model>>(), sol::base_classes, sol::bases<std::shared_ptr<Object>>());
	luaGltfModel = state.new_usertype<GltfModel>("GltfModel");

	luaModel.set_function("setgltfModel", &Model::setgltfModel);
}

void Scene::setFunctions()
{
	this->state.set_function("glueAddObject", &Scene::glueAddObject, this);
	this->state.set_function("glueAddModel", &Scene::glueAddModel, this);
	this->state.set_function("glueLoadModelResource", &Scene::glueLoadModelResource, this);
	this->state.set_function("glueBindCamera", &Scene::glueBindCamera, this);
}

void Scene::parseScene()
{

	script();
}

void Scene::glueAddObject(Object* obj)
{
}

void Scene::glueAddModel(Model* model)
{
	sceneSet.push_back(std::shared_ptr<Model>(model));
}

void Scene::glueBindCamera(Model* model)
{
	model->bindCamera(std::weak_ptr<Camera>(camera));
}

std::shared_ptr<GltfModel> Scene::glueLoadModelResource(OBJECT object)
{
	FileManager* fileManager = FileManager::GetInstance();
	std::shared_ptr<GltfModel> gltfModel = fileManager->loadModel(object);

	return gltfModel;
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