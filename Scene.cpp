#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)
{

	camera = std::make_shared<Camera>();

	initLuaScript(luaScriptPath);

	initFrameSetting();

	setModels();
}

Scene::Scene()
{
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
	lua_pushnumber(lua, (int)GLTFOBJECT::gltfTEST);
	lua_setglobal(lua, "gltfTEST");

	lua_pushnumber(lua, (int)GLTFOBJECT::CUBE);
	lua_setglobal(lua, "CUBE");
}

void Scene::registerFunctions()
{
	lua_register(lua, "glueCreateModel", glueCreateModel);
	lua_register(lua, "glueCreatePlayer", glueCreatePlayer);
	lua_register(lua, "glueSetLuaPath", glueSetLuaPath);
	lua_register(lua, "glueSetGltfModel",glueSetGltfModel);
	lua_register(lua, "glueSetPos", glueSetPos);
	lua_register(lua, "glueSetRotate", glueSetRotate);
	lua_register(lua, "glueSetScale", glueSetScale);
	lua_register(lua, "glueSetDiffuse", glueSetDiffuse);
	lua_register(lua, "glueBindCamera", glueBindCamera);
	lua_register(lua, "glueSetColider", glueSetColider);
	lua_register(lua, "glueSetColiderScale", glueSetColiderScale);
	lua_register(lua, "glueSetDefaultAnimationName", glueSetDefaultAnimationName);
	lua_register(lua, "glueSetGravity", glueSetGravity);
	lua_register(lua, "glueSetSlippery", glueSetSlippery);
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
		case ObjNum::MODEL:
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
		if (sceneSet[i]->getObjNum() == ObjNum::MODEL)
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
				case ObjNum::MODEL:
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
							model->setPosition(model->getPosition() + slopeCollision(collisionVector));
						}

						if (model2->isMovable)
						{
							//model2->setPosition(model2->getPosition() + up * glm::dot(collisionVector,up));
							model2->setPosition(model2->getPosition() + slopeCollision(collisionVector));
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
		case ObjNum::MODEL:
			model = std::dynamic_pointer_cast<Model>(sceneSet[i]);
			model->updateTransformMatrix();
			Storage::GetInstance()->addModel(model);
			break;
		default:
			continue;
		}
	}
}

glm::vec3 Scene::slopeCollision(glm::vec3 collisionVector)
{
	if (collisionVector.y > 0.5f)
	{
		return collisionVector * glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		return collisionVector;
	}
}

std::shared_ptr<Model> Scene::raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model)
{
	for (int i = 0; i < sceneSet.size(); i++)
	{
		std::shared_ptr<Model> model2;
		switch (sceneSet[i]->getObjNum())
		{
		case MODEL:
			model2 = std::dynamic_pointer_cast<Model>(sceneSet[i]);
			break;
		default:
			continue;
		}

		if (model == model2.get())
		{
			continue;
		}

		if (!model2->hasColider())
		{
			continue;
		}

		std::shared_ptr<Colider> colider2 = model2->getColider();
		if (colider2->Intersect(origin,dir,length))
		{
			return model2;
		}
	}

	return nullptr;
}