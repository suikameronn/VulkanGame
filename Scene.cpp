#include"Scene.h"

Scene::Scene()
{
}

Scene::Scene(FileManager* manager)
{
	//sceneSet = オブジェクトの名前と種類を読み取りシーンファイルをパスする
	std::vector<std::pair<std::string, OBJECT>> parth;

	int test = 100;
	parth.resize(test);
	for (int i = 0; i < test; i++)
	{
		parth[i].first = i;
		parth[i].second = MODELTEST;
	}

	//ファイルからモデルとテクスチャを読み取る
	for (auto itr = parth.begin(); itr != parth.end(); itr++)
	{
		sceneSet[itr->first] = *manager->loadModelPoints(itr->second);
		sceneSet[itr->first].setImageData(manager->loadModelImage(IMAGETEST));
	}

	setPointers();
}

Geometry* Scene::getSceneModelData(std::string name)
{
	return &sceneSet[name];
}

void Scene::setPointers()
{
	//描画するモデルのポインタを積んでいく
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		modelsPointer.push_back(&itr->second);
	}
}

void Scene::setPointers(std::string name)
{
	//シーン内の重複したモデルやデータを除いた、シーンに必要なモデルやテクスチャをmodelsPointerに入れる
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if (&itr->second == &sceneSet[name])
		{
			continue;
		}

		modelsPointer.push_back(&itr->second);
	}
}

std::vector<Geometry*>& Scene::getSceneModel()
{
	if (modelsPointer.empty())
	{
		std::cout << "getSceneModel error" << std::endl;
	}

	return modelsPointer;
}