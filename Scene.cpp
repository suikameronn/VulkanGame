#include"Scene.h"

Scene::Scene()
{
	camera = std::make_shared<Camera>();

	parseScene();

	setModels();
}

void Scene::parseScene()
{
	//sceneSet = オブジェクトの名前と種類を読み取りシーンファイルをパスする
	std::vector<std::pair<std::string, OBJECT>> parth;

	//VkDescriptorSetの上限は100
	int test = 1;
	parth.resize(test);
	for (int i = 0; i < test; i++)
	{
		parth[i].first = i;
		parth[i].second = OBJECT::gltfTEST;
	}

	{
		FileManager* fileManager = FileManager::GetInstance();

		
		std::shared_ptr<GltfModel> gltfModel = fileManager->loadModel(OBJECT::gltfTEST);
		std::shared_ptr<Player> model = std::shared_ptr<Player>(new Player());
		//モデルを読み込む関数
		model->setgltfModel(gltfModel);
		model->controllable = true;
		model->rotate.x = 0.0f;//単位は角度
		model->rotate.y = 0.0f;
		model->rotate.z = 0.0f;
		model->scale = glm::vec3(5.05f);
		model->controllable = true;
		model->setPosition(glm::vec3(0.0f,0.0f,0.0f));
		model->setColider();
		sceneSet["aaaaa"] = model;
		sceneSet["aaaaa"]->bindCamera(std::weak_ptr<Object>(camera));

		std::shared_ptr<GltfModel> gltfModel2 = fileManager->loadModel(OBJECT::CUBE);
		std::shared_ptr<Model> model2 = std::shared_ptr<Model>(new Player());
		//モデルを読み込む関数
		model2->setgltfModel(gltfModel2);
		model2->rotate.x = 0.0f;//単位は角度
		model2->rotate.y = 0.0f;
		model2->rotate.z = 0.0f;
		model2->scale = glm::vec3(5.05f);
		model2->controllable = true;
		model2->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		model2->setColider();
		sceneSet["a"] = model2;

		camera->spherePos = true;
	}
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
		for (auto itr2 = std::next(itr); itr2 != sceneSet.end(); itr2++)
		{
			if (itr->second->getColider()->Intersect(itr2->second->getColider(), collisionDepth, collisionVector))
			{
				itr->second->setPosition(itr->second->getPosition() + collisionVector * collisionDepth);
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

/*
void Scene::IntersectsColiders()
{
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if(!itr->second->hasColider())
		{
			continue;
		}
		std::shared_ptr<Colider> colider = itr->second->getColider();

		for (auto itr2 = sceneSet.begin(); itr2 != sceneSet.end(); itr2++)
		{
			if (itr == itr2)
			{
				continue;
			}

			if (!itr2->second->hasColider())
			{
				continue;
			}

			std::shared_ptr<Colider> oppColider = itr2->second->getColider();
			
			bool intersected = colider->Intersect(oppColider);
		}
	}
}
*/