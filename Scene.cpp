#include"Scene.h"

Scene::Scene()
{
	parseScene();

	setModels();
}

void Scene::parseScene()
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

	//�`�悷�郂�f���̃|�C���^��ς�ł���
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		VulkanBase::GetInstance()->setModels(itr->second);
	}

	VulkanBase::GetInstance()->endFirstSendModel();
}

void Scene::setModels(std::string name)
{
	//�V�[�����̏d���������f����f�[�^���������A�V�[���ɕK�v�ȃ��f����e�N�X�`����modelsPointer�ɓ����
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if (&itr->second == &sceneSet[name])
		{
			continue;
		}

		VulkanBase::GetInstance()->setModels(itr->second);
	}
}