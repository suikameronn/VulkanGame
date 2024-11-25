#include"Scene.h"

Scene::Scene()
{
	camera = std::make_shared<Camera>();

	parseScene();

	setModels();
}

void Scene::parseScene()
{
	//sceneSet = �I�u�W�F�N�g�̖��O�Ǝ�ނ�ǂݎ��V�[���t�@�C�����p�X����
	std::vector<std::pair<std::string, OBJECT>> parth;

	//VkDescriptorSet�̏����100
	int test = 1;
	parth.resize(test);
	for (int i = 0; i < test; i++)
	{
		parth[i].first = i;
		parth[i].second = OBJECT::FBXTEST;
	}

	{
		FileManager* fileManager = FileManager::GetInstance();

		
		std::shared_ptr<GltfModel> fbxModel = fileManager->loadModel(OBJECT::UNITYCHAN_NO_ANIM);
		//fbxModel->setAnimation(ACTION::IDLE, fileManager->loadAnimations(fbxModel.get(), OBJECT::IDLE));
		//fbxModel->setAnimation(ACTION::WALK, fileManager->loadAnimations(fbxModel.get(), OBJECT::WALK));
		std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
		//���f����ǂݍ��ފ֐�
		model->setFbxModel(fbxModel);
		model->controllable = true;
		model->rotate.x = 0.0f;//�P�ʂ͊p�x
		model->rotate.y = 0.0f;
		model->rotate.z = 0.0f;
		//model->scale = glm::vec3(3.2, 3.2, 3.2);
		model->scale = glm::vec3(10.05f);
		model->controllable = true;
		//model->setColider(BOX,1.0f, 0.37f, 3.7f, 0.0f, 0.56f, 0.56f);
		//model->setColider(BOX,glm::vec3(10.4,11.0,11.0));
		model->setPosition(glm::vec3(0.0f,0.0f,0.0f));
		sceneSet["aaaaa"] = model;
		sceneSet["aaaaa"]->bindCamera(std::weak_ptr<Object>(camera));

		camera->spherePos = true;
		
		/*
		std::shared_ptr<Model> m = std::shared_ptr<Model>(new Model());
		fbxModel = fileManager->loadModel(OBJECT::NORMALBOX);
		m->setFbxModel(fbxModel);
		m->scale = glm::vec3(10.0f);
		m->setColider(BOX);
		m->setPosition(glm::vec3(0.0f,0.0f,0.0f));
		sceneSet["aa"] = m;
		*/
	}

	/*
	//�t�@�C�����烂�f���ƃe�N�X�`����ǂݎ��
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

	/*
	if (sceneSet["aa"]->getColider()->Intersect(sceneSet["aaaaa"]->getColider(),collisionDepth,collisionVector))
	{
		sceneSet["aa"]->setPosition(sceneSet["aa"]->getPosition() + collisionVector * collisionDepth);
	}
	*/

	/*
	if (sceneSet["aaaaa"]->getColider()->Intersect(sceneSet["aa"]->getColider(), collisionDepth, collisionVector))
	{
		sceneSet["aaaaa"]->setPosition(sceneSet["aaaaa"]->getPosition() + collisionVector * collisionDepth);
	}
	*/

	for (auto itr = sceneSet.begin(); itr != std::prev(sceneSet.end()); itr++)
	{
		for (auto itr2 = std::next(itr); itr2 != sceneSet.end(); itr2++)
		{
			if (itr->second->getColider()->Intersect(itr2->second->getColider(), collisionDepth, collisionVector))
			{
				itr->second->setPosition(itr->second->getPosition() + collisionVector * collisionDepth);
			}
		}
	}

	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		if (itr->second->uniformBufferChange)
		{
			itr->second->updateTransformMatrix();
		}
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

	//�`�悷�郂�f���̃|�C���^��ς�ł���
	for (auto itr = sceneSet.begin(); itr != sceneSet.end(); itr++)
	{
		itr->second->updateTransformMatrix();
		Storage::GetInstance()->addModel(itr->second);
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