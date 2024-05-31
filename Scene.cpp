#include"Scene.h"

Scene::Scene()
{
	player = std::make_unique<Player>();
	camera = std::make_unique<Camera>();

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
		parth[i].second = MODELTEST;
	}

	{
		Model* model = new Model();
		model->setMeshes(FileManager::GetInstance()->loadModelPoints(MODELTEST));
		model->setImageData(FileManager::GetInstance()->loadModelImage(IMAGETEST));
		player->setObject(model);

		camera->bindObject(model);
	}

	//ファイルからモデルとテクスチャを読み取る
	for (auto itr = parth.begin(); itr != parth.end(); itr++)
	{
		Model* model = new Model();
		model->setMeshes(FileManager::GetInstance()->loadModelPoints(itr->second));
		model->setImageData(FileManager::GetInstance()->loadModelImage(IMAGETEST));

		sceneSet[itr->first] = model;
	}
}

bool Scene::UpdateScene()
{
	bool exit = false;

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->Update();
	}

	player->Update();
	camera->Update();

	return exit;
}

Model* Scene::getSceneModelData(std::string name)
{
	return static_cast<Model*>(sceneSet[name]);
}

void Scene::setModels()
{
	Storage::GetInstance()->setCamera(camera.get());

	//描画するモデルのポインタを積んでいく
	Storage::GetInstance()->addModel(static_cast<Model*>(player->getObject()));
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		Storage::GetInstance()->addModel(static_cast<Model*>(itr->second));
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

		Storage::GetInstance()->addModel(static_cast<Model*>(itr->second));
	}
}