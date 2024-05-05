#include"Scene.h"

Scene::Scene()
{
}

Scene::Scene(std::shared_ptr<FileManager> manager)
{
	//sceneSet = オブジェクトの名前と種類を読み取りシーンファイルをパスする
	std::vector<std::pair<std::string, OBJECT>> parth;
	std::pair<std::string, OBJECT> pair;
	pair.first = "test";
	pair.second = TEST;
	parth.push_back(pair);

	for (auto itr = parth.begin(); itr != parth.end(); itr++)
	{
		sceneSet[itr->first] = manager->loadModel(itr->second);
	}

	setPointers();
}

std::shared_ptr<Model> Scene::getSceneModelData(std::string name)
{
	return sceneSet[name];
}

void Scene::setPointers()
{
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		modelsPointer.push_back(itr->second);
	}
}

void Scene::setPointers(std::string name)
{
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if (itr->second == sceneSet[name])
		{
			continue;
		}

		modelsPointer.push_back(itr->second);
	}
}

std::vector<std::shared_ptr<Model>>& Scene::getSceneModel()
{
	if (modelsPointer.empty())
	{
		std::cout << "error" << std::endl;
	}

	return modelsPointer;
}