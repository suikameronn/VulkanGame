#include"Scene.h"

Scene::Scene()
{
}

Scene::Scene(FileManager* manager)
{
	//sceneSet = �I�u�W�F�N�g�̖��O�Ǝ�ނ�ǂݎ��V�[���t�@�C�����p�X����
	std::vector<std::pair<std::string, OBJECT>> parth;
	std::pair<std::string, OBJECT> pair;
	pair.first = "test";
	pair.second = TEST;
	parth.push_back(pair);

	for (auto itr = parth.begin(); itr != parth.end(); itr++)
	{
		sceneSet[itr->first] = *manager->loadModel(itr->second);
	}

	setPointers();
}

Model* Scene::getSceneModelData(std::string name)
{
	return &sceneSet[name];
}

void Scene::setPointers()
{
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		modelsPointer.push_back(&itr->second);
	}
}

void Scene::setPointers(std::string name)
{
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if (&itr->second == &sceneSet[name])
		{
			continue;
		}

		modelsPointer.push_back(&itr->second);
	}
}

std::vector<Model*>& Scene::getSceneModel()
{
	if (modelsPointer.empty())
	{
		std::cout << "getSceneModel error" << std::endl;
	}

	return modelsPointer;
}