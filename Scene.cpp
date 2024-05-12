#include"Scene.h"

Scene::Scene()
{
}

Scene::Scene(FileManager* manager)
{
	//sceneSet = �I�u�W�F�N�g�̖��O�Ǝ�ނ�ǂݎ��V�[���t�@�C�����p�X����
	std::vector<std::pair<std::string, OBJECT>> parth;

	int test = 100;
	parth.resize(test);
	for (int i = 0; i < test; i++)
	{
		parth[i].first = i;
		parth[i].second = MODELTEST;
	}

	//�t�@�C�����烂�f���ƃe�N�X�`����ǂݎ��
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
	//�`�悷�郂�f���̃|�C���^��ς�ł���
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		modelsPointer.push_back(&itr->second);
	}
}

void Scene::setPointers(std::string name)
{
	//�V�[�����̏d���������f����f�[�^���������A�V�[���ɕK�v�ȃ��f����e�N�X�`����modelsPointer�ɓ����
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