#include"Scene.h"

Scene::Scene()
{
}

void Scene::init(std::string luaScriptPath)
{

	camera = std::make_shared<Camera>();

	initLuaScript(luaScriptPath);

	initFrameSetting();

	setModels();
}

Scene::~Scene()
{
}

void Scene::initFrameSetting()
{
	for (int i = 0; i < sceneSet.size(); i++)
	{
		switch (sceneSet[i]->getObjNum())
		{
		case 0:
			sceneSet[i]->initFrameSetting();
			break;
		case 1:
			std::shared_ptr<Model> model = std::dynamic_pointer_cast<Model>(sceneSet[i]);
			model->initFrameSetting();
			break;
		}
	}
}

void Scene::initLuaScript(std::string path)
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);
	lua_setglobal(lua, "Scene");

	registerOBJECT();
	registerFunctions();

	luaL_dofile(lua,path.c_str());
}

void Scene::registerOBJECT()
{
	lua_pushnumber(lua, (int)OBJECT::gltfTEST);
	lua_setglobal(lua, "gltfTEST");

	lua_pushnumber(lua, (int)OBJECT::CUBE);
	lua_setglobal(lua, "CUBE");
}

void Scene::registerFunctions()
{
	lua_register(lua, "glueCreateModel", glueCreateModel);
	lua_register(lua, "glueCreatePlayer", glueCreatePlayer);
	lua_register(lua, "glueSetGltfModel",glueSetGltfModel);
	lua_register(lua, "glueSetPos", glueSetPos);
	lua_register(lua, "glueSetRotate", glueSetRotate);
	lua_register(lua, "glueSetScale", glueSetScale);
	lua_register(lua, "glueSetDiffuse", glueSetDiffuse);
	lua_register(lua, "glueBindCamera", glueBindCamera);
	lua_register(lua, "glueSetColider", glueSetColider);
	lua_register(lua, "glueSetColiderScale", glueSetColiderScale);
	lua_register(lua, "glueSetDefaultAnimationName", glueSetDefaultAnimationName);
}

bool Scene::UpdateScene()
{
	bool exit = false;

	camera->Update();

	for (int i = 0; i < sceneSet.size(); i++)
	{
		std::shared_ptr<Model> model;
		switch (sceneSet[i]->getObjNum())
		{
		case ObjNum::cModel:
			model = std::dynamic_pointer_cast<Model>(sceneSet[i]);
			model->Update();
			break;
		default:
			continue;
		}
	}

	for (int i = 0; i < sceneSet.size(); i++)
	{
		if (sceneSet[i]->uniformBufferChange)
		{
			sceneSet[i]->updateTransformMatrix();
		}
	}

	for(int i = 0;i < sceneSet.size() - 1;i++)
	{
		if (sceneSet[i]->getObjNum() == ObjNum::cModel)
		{
			std::shared_ptr<Model> model = std::dynamic_pointer_cast<Model>(sceneSet[i]);

			if (!model->hasColider())
			{
				continue;
			}

			for (int j = i + 1; j < sceneSet.size(); j++)
			{
				std::shared_ptr<Model> model2;
				switch (sceneSet[j]->getObjNum())
				{
				case ObjNum::cModel:
					model2 = std::dynamic_pointer_cast<Model>(sceneSet[j]);
					break;
				default:
					continue;
				}

				if (model2->hasColider())
				{
					if (model->getColider()->Intersect(model2->getColider(), collisionDepth, collisionVector))
					{
						if (model->isMovable)
						{
							model->setPosition(model->getPosition() + collisionVector * collisionDepth);
						}

						if (model2->isMovable)
						{
							model2->setPosition(model2->getPosition() + (-collisionVector) * collisionDepth);
						}
					}
				}
			}
		}
	}

	for (int i = 0;i < sceneSet.size();i++)
	{
		if (sceneSet[i]->uniformBufferChange)
		{
			sceneSet[i]->updateTransformMatrix();
		}
	}

	return exit;
}

void Scene::setModels()
{
	Storage::GetInstance()->setCamera(camera);

	//描画するモデルのポインタを積んでいく
	std::shared_ptr<Model> model;
	for (int i = 0;i < sceneSet.size();i++)
	{
		switch (sceneSet[i]->getObjNum())
		{
		case ObjNum::cModel:
			model = std::dynamic_pointer_cast<Model>(sceneSet[i]);
			model->updateTransformMatrix();
			Storage::GetInstance()->addModel(model);
			break;
		default:
			continue;
		}
	}
}