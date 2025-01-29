#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)
{

	camera = std::make_shared<Camera>();

	initLuaScript(luaScriptPath);

	initFrameSetting();

	prepareRenderData();
}

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::initFrameSetting()
{
	for (int i = 0; i < sceneModels.size(); i++)
	{
		sceneModels[i]->initFrameSetting();
	}
	player->initFrameSetting();
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
	lua_register(lua, "glueCreatePointLight", glueCreatePointLight);
	lua_register(lua, "glueSetLightColor", glueSetLightColor);
	lua_register(lua, "glueCreateDirectionalLight", glueCreateDirectionalLight);
	lua_register(lua, "glueSetLightDirection", glueSetLightDirection);
	lua_register(lua, "glueBindObject", glueBindObject);
}

bool Scene::UpdateScene()
{
	bool exit = false;

	camera->Update();

	player->Update();

	for (int i = 0; i < sceneModels.size(); i++)
	{
		sceneModels[i]->Update();

		if (sceneModels[i]->uniformBufferChange)
		{
			sceneModels[i]->updateTransformMatrix();
		}
	}

	for (int i = 0; i < sceneModels.size() - 1; i++)
	{
		if (!sceneModels[i]->hasColider())
		{
			continue;
		}

		for (int j = i + 1; j < sceneModels.size(); j++)
		{
			if (sceneModels[j]->hasColider())
			{
				if (sceneModels[i]->getColider()->Intersect(sceneModels[j]->getColider(), collisionDepth, collisionVector))
				{
					if (sceneModels[i]->isMovable)
					{
						sceneModels[i]->setPosition(sceneModels[i]->getPosition() + slopeCollision(collisionVector));
					}

					if (sceneModels[j]->isMovable)
					{
						sceneModels[j]->setPosition(sceneModels[j]->getPosition() + slopeCollision(collisionVector));
					}
				}
			}
		}
	}

	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (!sceneModels[i]->hasColider())
		{
			continue;
		}

		if (player->getColider()->Intersect(sceneModels[i]->getColider(), collisionDepth, collisionVector))
		{
			if (sceneModels[i]->isMovable)
			{
				sceneModels[i]->setPosition(sceneModels[i]->getPosition() + slopeCollision(collisionVector));
			}

			if (player->isMovable)
			{
				player->setPosition(player->getPosition() + slopeCollision(-collisionVector));
			}
		}
	}

	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (sceneModels[i]->uniformBufferChange)
		{
			sceneModels[i]->updateTransformMatrix();
		}
	}
	player->updateTransformMatrix();

	return exit;
}

void Scene::prepareRenderData()
{
	setLights();
	setModels();
}

void Scene::setModels()
{
	Storage::GetInstance()->setCamera(camera);

	//描画するモデルのポインタを積んでいく
	for (int i = 0;i < sceneModels.size();i++)
	{
		sceneModels[i]->updateTransformMatrix();
		Storage::GetInstance()->addModel(sceneModels[i]);
	}

	player->updateTransformMatrix();
	Storage::GetInstance()->addModel(player);
}

void Scene::setLights()
{
	for (std::shared_ptr<PointLight> pl : scenePointLights)
	{
		pl->updateTransformMatrix();
		Storage::GetInstance()->addLight(pl);
	}

	for (std::shared_ptr<DirectionalLight> dl : sceneDirectionalLights)
	{
		dl->updateTransformMatrix();
		Storage::GetInstance()->addLight(dl);
	}

	//LightにVulkanでの変数などを持たせる
	Storage::GetInstance()->prepareLightsForVulkan();
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
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (model == sceneModels[i].get())
		{
			continue;
		}

		if (!sceneModels[i]->hasColider())
		{
			continue;
		}

		std::shared_ptr<Colider> colider2 = sceneModels[i]->getColider();
		if (colider2->Intersect(origin,dir,length))
		{
			return sceneModels[i];
		}
	}

	return nullptr;
}