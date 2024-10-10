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
		parth[i].second = OBJECT::FBXTEST;
	}

	{
		FileManager* fileManager = FileManager::GetInstance();

		
		std::shared_ptr<FbxModel> fbxModel = fileManager->loadModel(OBJECT::UNITYCHAN_NO_ANIM);
		fbxModel->setAnimation(ACTION::IDLE, fileManager->loadAnimations(fbxModel.get(), OBJECT::IDLE));
		fbxModel->setAnimation(ACTION::WALK, fileManager->loadAnimations(fbxModel.get(), OBJECT::WALK));
		std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
		//モデルを読み込む関数
		model->setFbxModel(fbxModel);
		model->controllable = true;
		model->rotate.x = 0.0f;//単位は角度
		model->rotate.y = 0.0f;
		model->rotate.z = 0.0f;
		model->scale = glm::vec3(3.2, 3.2, 3.2);
		model->controllable = true;
		model->setColider(BOX,1.0f, 0.37f, 3.7f, 0.0f, 0.56f, 0.56f);
		model->setPosition(glm::vec3(50.1f,50.1f,50.1f));
		sceneSet["aaaaa"] = model;
		sceneSet["aaaaa"]->bindCamera(std::weak_ptr<Object>(camera));

		camera->spherePos = true;
		
		
		std::shared_ptr<Model> m = std::shared_ptr<Model>(new Model());
		m->setFbxModel(FileManager::GetInstance()->loadModel(OBJECT::NORMALBOX));
		m->scale = glm::vec3(10.0f, 30.0f, 10.0f);
		m->setPosition(glm::vec3(0.0f,0.0f,0.0f));
		m->setColider(BOX);
		sceneSet["aa"] = m;
	}

	/*
	//ファイルからモデルとテクスチャを読み取る
	for (auto itr = parth.begin(); itr != parth.end(); itr++)
	{
		Model* model = new Model();
		model->setMeshes(FileManager::GetInstance()->loadModelPoints(itr->second));
		model->setImageData(FileManager::GetInstance()->loadModelImage(IMAGETEST));

		sceneSet[itr->first] = model;
	}
	*/
}

bool Scene::UpdateScene()
{
	bool exit = false;

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->Update();
	}

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->updateTransformMatrix();
	}

	camera->Update();

	if (sceneSet["aaaaa"]->getColider()->Intersect(sceneSet["aa"]->getColider()))
	{
		std::cout << "aaaaaaaaaaaaaaaaaaaaaa" << std::endl;
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