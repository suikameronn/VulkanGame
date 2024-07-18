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
		parth[i].second = FBXTEST;
	}

	{
		std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
		model->setFbxModel(FileManager::GetInstance()->loadModel(UNITYCHAN_NO_ANIM));
		//モデルを読み込む関数
		model->controllable = true;
		sceneSet["aaaaa"] = model;
		sceneSet["aaaaa"]->bindObject(camera);
		//camera->spherePos = true;

		/*
		std::shared_ptr<Model> m = std::shared_ptr<Model>(new Model());
		m->setFbxModel(FileManager::GetInstance()->loadModel(GROUND1));
		m->scale = glm::vec3(10, 30, 10);
		sceneSet["aa"] = m;
		*/
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

	camera->Update();

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->updateTransformMatrix();
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