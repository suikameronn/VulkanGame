#include"Scene.h"

Scene::Scene()
{
	player = std::make_unique<Player>();
	parseScene();

	setModels();
}

void Scene::parseScene()
{
	//sceneSet = �I�u�W�F�N�g�̖��O�Ǝ�ނ�ǂݎ��V�[���t�@�C�����p�X����
	std::vector<std::pair<std::string, OBJECT>> parth;

	//VkDescriptorSet�̏����100
	int test = 0;
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

bool Scene::UpdateScene()
{
	bool exit = false;

	player->Update();

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->Update();
	}

	return exit;
}

Model* Scene::getSceneModelData(std::string name)
{
	return static_cast<Model*>(sceneSet[name]);
}

void Scene::setModels()
{
	//�`�悷�郂�f���̃|�C���^��ς�ł���
	Storage::GetInstance()->addModel(static_cast<Model*>(player->getObject()));

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		Storage::GetInstance()->addModel(static_cast<Model*>(itr->second));
	}
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

		Storage::GetInstance()->addModel(static_cast<Model*>(itr->second));
	}
}