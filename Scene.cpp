#include"Scene.h"

Scene::Scene()
{
	parseScene();

	setModels();
}

void Scene::parseScene()
{
	//sceneSet = オブジェクトの名前と種類を読み取りシーンファイルをパスする
	std::vector<std::pair<std::string, OBJECT>> parth;

	int test = 1;
	parth.resize(test);
	for (int i = 0; i < test; i++)
	{
		parth[i].first = i;
		parth[i].second = MODELTEST;
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

Model* Scene::getSceneModelData(std::string name)
{
	return sceneSet[name];
}

void Scene::setModels()
{
	VulkanBase::GetInstance()->resizeModels(sceneSet.size());

	//描画するモデルのポインタを積んでいく
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		VulkanBase::GetInstance()->setModels(itr->second);
	}

	VulkanBase::GetInstance()->endFirstSendModel();
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

		VulkanBase::GetInstance()->setModels(itr->second);
	}
}