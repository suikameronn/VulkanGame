#include"GameManager.h"

#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)//lua�t�@�C���̃p�X���󂯎��
{
	startPoint = glm::vec3(0.0f);

	GameManager* manager = GameManager::GetInstance();

	camera = std::shared_ptr<Camera>(new Camera(manager->getWindowWidth(), manager->getWindowHeight()));
	Storage::GetInstance()->setCamera(camera);

	rtree = std::make_unique<RTree<Model>>();

	cubemap = std::shared_ptr<Cubemap>(new Cubemap());

	initLuaScript(luaScriptPath);//lua����X�e�[�W�̃f�[�^��ǂݎ��

	initFrameSetting();//lua����ǂݎ�����I�u�W�F�N�g�̏���������
}

Scene::Scene()
{
}

Scene::~Scene()
{
	cleanupVulkan();
}

//����t���[���̂ݎ��s �X�e�[�W��̂��ׂẴI�u�W�F�N�g�̏���t���[�����̐ݒ���s��
void Scene::initFrameSetting()
{
	Storage* storage = Storage::GetInstance();

	FileManager::GetInstance()->addLoadModelList("models/cubemap.glb", cubemap.get());

	//Model�N���X��gltfModel�N���X��ݒ肷��
	FileManager::GetInstance()->setGltfModel();

	VulkanBase* vulkan = VulkanBase::GetInstance();

	for (auto itr = sceneModels.begin(); itr != sceneModels.end();itr++)//���ׂẴI�u�W�F�N�g�̏���������
	{
		(*itr)->initFrameSetting();//�I�u�W�F�N�g�̏���������
		vulkan->setModelData(*itr);
	}

	player->initFrameSetting();//�v���C���[�N���X�̂ݕʘg
	player->setPosition(startPoint);//�v���C���[�������ʒu��
	vulkan->setModelData(player);

	vulkan->createCubemap(cubemap);

	for (auto itr = sceneUI.begin(); itr != sceneUI.end(); itr++)
	{
		(*itr)->initFrameSettings();
		vulkan->setUI(*itr);
	}

	//�V���h�E�}�b�s���O�̗p��
	vulkan->prepareShadowMapping(static_cast<int>(sceneDirectionalLights.size()), shadowMapData);

	//���C�g�̃o�b�t�@�̗p��
	vulkan->createPointLightBuffer(pointLightBuffer);
	vulkan->createDirectionalLightBuffer(dirLightBuffer);
}

//lua�X�N���v�g�̓ǂݎ��A���s
void Scene::initLuaScript(std::string path)
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);//����Scene�̃C���X�^���X��lua�̉��z�}�V���ɑ���
	lua_setglobal(lua, "Scene");//�C���X�^���X��lua�X�N���v�g�Ŏg����悤�ɂ���

	registerFunctions();//�X�N���v�g����Ăяo���֐���o�^����

	luaL_dofile(lua,path.c_str());//�X�N���v�g�̎��s

	lua_close(lua);
}

void Scene::registerFunctions()//lua����Ăяo�����ÓI�֐���ݒ�
{
	lua_register(lua, "loadGltfModel", glueSceneFunction::glueLoadGltfModel);//Gltf���f���̓ǂݍ��݂݂̂��s��
	lua_register(lua, "createModel", glueSceneFunction::glueCreateModel);//3D���f���N���X�̒ǉ�
	lua_register(lua, "createPlayer", glueSceneFunction::glueCreatePlayer);//�v���C���[�̒ǉ�
	lua_register(lua, "setLuaPath", glueSceneFunction::glueSetLuaPath);//�X�N���v�g���I�u�W�F�N�g�ɒǉ�
	lua_register(lua, "setDelayStartLua", glueSceneFunction::glueSetDelayStartLua);//�X�N���v�g�̎��s�J�n�^�C�~���O��ݒ�
	lua_register(lua, "setGltfModel", glueSceneFunction::glueSetGltfModel);//gltf���f���̒ǉ����A�I�u�W�F�N�g�ɐݒ肷��
	lua_register(lua, "setPos", glueSceneFunction::glueSetPos);//���W�̐ݒ�
	lua_register(lua, "setRotate", glueSceneFunction::glueSetRotate);//������ݒ�
	lua_register(lua, "setScale", glueSceneFunction::glueSetScale);//���f���̃X�P�[���̐ݒ�
	lua_register(lua, "setBaseColor", glueSceneFunction::glueSetBaseColor);//Diffuse�J���[�̐ݒ�
	lua_register(lua, "bindCamera", glueSceneFunction::glueBindCamera);//�J�������Ǐ]����I�u�W�F�N�g��ݒ肷��
	lua_register(lua, "setAABBColider", glueSceneFunction::glueSetAABBColider);//���f����AABB�R���C�_�[��t����
	lua_register(lua, "setColiderScale", glueSceneFunction::glueSetColiderScale);//�R���C�_�[�̃X�P�[����ݒ�
	lua_register(lua, "setDefaultAnimationName", glueSceneFunction::glueSetDefaultAnimationName);//���f���Ƀf�t�H���g�̃A�j���[�V������ݒ肷��
	lua_register(lua, "setGravity", glueSceneFunction::glueSetGravity);//�I�u�W�F�N�g�ɏd�ʂ���������
	lua_register(lua, "createPointLight", glueSceneFunction::glueCreatePointLight);//�|�C���g���C�g���쐬
	lua_register(lua, "setLightColor", glueSceneFunction::glueSetLightColor);//���C�g�̃J���[��ݒ�
	lua_register(lua, "createDirectionalLight", glueSceneFunction::glueCreateDirectionalLight);//���s�������쐬
	lua_register(lua, "setLightDirection", glueSceneFunction::glueSetLightDirection);//���s�����̕�����ݒ�
	lua_register(lua, "bindObject", glueSceneFunction::glueBindObject);//�I�u�W�F�N�g�ɐe�q�֌W��ݒ肷��
	lua_register(lua, "setStartPoint", glueSceneFunction::glueSetStartPoint);//�������W�̐ݒ�
	lua_register(lua, "setLimitY", glueSceneFunction::glueSetLimitY);//y���W�̉����̐ݒ�
	lua_register(lua, "setHDRIMap", glueSceneFunction::glueSetHDRIMap);//HDRI�摜�̐ݒ�
	lua_register(lua, "setUVScale", glueSceneFunction::glueSetUVScale);//uv�𒲐�����悤�ɂ��āA�e�N�X�`���̈������΂���h��
	lua_register(lua, "setTransparent", glueSceneFunction::glueSetTransparent);//�������̕\���̃t���b�O��ݒ�
}

//�������W�̐ݒ�
void Scene::setStartPoint(glm::vec3 point)
{
	startPoint = point;
}

//y���W�̉����̐ݒ�
void Scene::setLimitY(float y)
{
	limitY = y;
}

//HDRI�}�b�v�̐ݒ�
void Scene::setHDRIMap(std::string imagePath)
{
	cubemap->setHDRIMap(FileManager::GetInstance()->loadImage(imagePath));
}

//�I�u�W�F�N�g�̐ڒn����Ȃǂ����Z�b�g
void Scene::resetStatus()
{
	for (auto itr = sceneModels.begin(); itr != sceneModels.end(); itr++)
	{
		(*itr)->clearGroundingObject();//�ڒn����Ă���I�u�W�F�N�g�̓o�^�̏�����
		(*itr)->isGrounding = false;
	}

	player->isGrounding = false;
}

int Scene::UpdateScene()//�X�e�[�W��̃I�u�W�F�N�g�Ȃǂ̍X�V����
{
	int exit = GAME_CONTINUE;

	camera->Update();//�J�����̍X�V����
	camera->updateTransformMatrix();

	player->Update();//�v���C���[�̍X�V����
	player->updateTransformMatrix();//�v���C���[�̍��W�ϊ��s��̍X�V

	for (auto itr = sceneModels.begin(); itr != sceneModels.end();)//���ׂẴI�u�W�F�N�g�̍X�V����
	{
		//�I�u�W�F�N�g�̍X�V����
		(*itr)->Update();

		if (!(*itr)->isExist())
		{
			//�I�u�W�F�N�g���폜����
			itr = sceneModels.erase(itr);

			continue;
		}

		if ((*itr)->uniformBufferChange)//�I�u�W�F�N�g�̈ʒu��������ς�����ꍇ
		{
			(*itr)->updateTransformMatrix();//MVP�̃��f���s����X�V����
		}

		itr++;
	}

	resetStatus();//�V�[���S�̂̃I�u�W�F�N�g�̃��Z�b�g�������s��

	//�����蔻��
	rtreeIntersect();
	//intersect();

	//�����v���C���[���X�e�[�W�̉����𒴂�����A���X�^�[�g������
	if (player->getPosition().y < limitY)
	{
		player->restart(startPoint);
	}

	//�t���[���I�����ɁA�o�b�t�@�̍X�V�Ȃǂ�����
	frameEnd();

	if (Controller::GetInstance()->getKey(GLFW_KEY_ESCAPE))
	{
		exit = GAME_FINISH;
	}

	//�O�̃t���[���̔j���\��̃o�b�t�@��j������
	VulkanBase::GetInstance()->cleanupDefferedBuffer();
	
	//�����_�����O
	render();

	return exit;//�E�B���h�E����悤�Ƃ����ꍇ�́Afals�𑗂�A�Q�[���̏I�������������� 
}

//�R���C�_�[�����ɐڒn���Ă��邩�̔��� true�̏ꍇ�͂��̏��̈ړ���Ǐ]����
bool Scene::groundCollision(glm::vec3 collisionVector)
{
	glm::vec3 v = -glm::normalize(collisionVector);

	float dot = glm::dot(v, up);

	return dot >= 0;
}

//�l�p�`�����΂��āA�Փ˔�����s���A�ڒn����Ɏg����
std::weak_ptr<Model> Scene::raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model,glm::vec3& normal)
{
	for (auto itr = sceneModels.begin();itr != sceneModels.end();itr++)
	{
		if (model == (itr->get()))
		{
			continue;
		}

		if (!(*itr)->hasColider())
		{
			continue;
		}

		std::shared_ptr<Colider> colider2 = (*itr)->getColider();
		if (colider2->Intersect(origin,dir,length,normal))//�R���C�_�[�Ɛ����̏Փ˔���
		{
			return (*itr);
		}
	}

	return std::weak_ptr<Model>();
}

//�V�[���S�̂�R�؂ɃI�u�W�F�N�g��ǉ�����
void Scene::addModelToRTree(std::weak_ptr<Model> model)
{
	rtree->insert(model, model.lock()->getMbrMin(), model.lock()->getMbrMax());
}

//�c���[���̃I�u�W�F�N�g�̈ʒu���X�V����
void Scene::updateObjectPos(std::weak_ptr<Model> model, RNode<Model>* node)
{
	rtree->reflectMove(model, node);
}

void Scene::rtreeIntersect()
{
	for (auto itr = sceneModels.begin(); itr != sceneModels.end();itr++)//���f�����m�̓����蔻����s��
	{
		if (!(*itr)->hasColider())//�R���C�_�[�������Ă��Ȃ�������X�L�b�v
		{
			continue;
		}

		//R�؂��g���āA�����蔻����s���I�u�W�F�N�g���i��
		std::vector<std::weak_ptr<Model>> collisionDetectTarget;
		rtree->broadPhaseCollisionDetection(collisionDetectTarget, (*itr)->getMbrMin(), (*itr)->getMbrMax());

		for (int j = 0; j < collisionDetectTarget.size(); j++)
		{
			std::shared_ptr<Model> target = collisionDetectTarget[j].lock();

			std::shared_ptr<Colider> itrColider = (*itr)->getColider();
			std::shared_ptr<Colider> targetColider = target->getColider();

			//�R���C�_�[���ݒ肳��Ă��邩�A�������g�������I�u�W�F�N�g�Ɠ����蔻����s��
			if (targetColider && (*itr) == target)
			{

				if (itrColider->Intersect(targetColider, collisionVector))//GJK�ɂ�铖���蔻����s��
				{
					if ((*itr)->isMovable && !targetColider->isTrigger())//�ǂȂǓ������Ȃ����̂͏��O����
					{
						(*itr)->setPosition((*itr)->getPosition() + collisionVector);//�Փ˂���������
						if (groundCollision(collisionVector))//������ڂ̃I�u�W�F�N�g����߂̃I�u�W�F�N�g�̐^��ɂ���
							//�܂�A���̂悤�ȃI�u�W�F�N�g�̏�ɍڂ��Ă���󋵂̏ꍇ
						{
							target->isGrounding = true;
							(*itr)->addGroundingObject(target);//���̃I�u�W�F�N�g�̈ړ��ɁA��ɍڂ��Ă���I�u�W�F�N�g��Ǐ]������
						}
					}

					if (target->isMovable && !itrColider->isTrigger())//��Ɠ��l
					{
						target->setPosition(target->getPosition() - collisionVector);
						if (groundCollision(-collisionVector))
						{
							(*itr)->isGrounding = true;
							target->addGroundingObject((*itr));
						}
					}
				}
			}
		}
	}

	{//�v���C���[�L�����N�^�[�ɂ��Ă̓����蔻����s��
		std::vector<std::weak_ptr<Model>> collisionDetectTarget;
		rtree->broadPhaseCollisionDetection(collisionDetectTarget, player->getMbrMin(), player->getMbrMax());

		for (int i = 0; i < collisionDetectTarget.size(); i++)
		{
			std::shared_ptr<Model> target = collisionDetectTarget[i].lock();

			std::shared_ptr<Colider> playerColider = player->getColider();
			std::shared_ptr<Colider> targetColider = target->getColider();

			if (targetColider)
			{
				if (playerColider->Intersect(targetColider, collisionVector))
				{
					if (target->isMovable && !playerColider->isTrigger())
					{
						target->setPosition(target->getPosition() + collisionVector);
					}

					if (player->isMovable && !targetColider->isTrigger())
					{
						if (groundCollision(collisionVector))
						{
							player->isGrounding = true;
							target->addGroundingObject(player);
						}

						player->setPosition(player->getPosition() - collisionVector);
					}
				}
			}
		}
	}
}

void Scene::intersect()
{
	for (auto itr = sceneModels.begin(); itr != sceneModels.end();itr++)//���f�����m�̓����蔻����s��
	{
		if (!(*itr)->hasColider())//�R���C�_�[�������Ă��Ȃ�������X�L�b�v
		{
			continue;
		}

		auto nextItr = std::next(itr);
		for (auto opp = nextItr; opp != sceneModels.end();opp++)
		{
			if (!(*opp)->hasColider())
			{
				continue;
			}

			//�R���C�_�[���ݒ肳��Ă��邩�A�������g�������I�u�W�F�N�g�Ɠ����蔻����s��
			if ((*itr)->getColider()->Intersect((*opp)->getColider(), collisionVector))//GJK�ɂ�铖���蔻����s��
			{
				if ((*itr)->isMovable)//�ǂȂǓ������Ȃ����̂͏��O����
				{
					(*itr)->setPosition((*itr)->getPosition() + collisionVector);//�Փ˂���������
					if (groundCollision(collisionVector))//������ڂ̃I�u�W�F�N�g����߂̃I�u�W�F�N�g�̐^��ɂ���
						//�܂�A���̂悤�ȃI�u�W�F�N�g�̏�ɍڂ��Ă���󋵂̏ꍇ
					{
						(*opp)->isGrounding = true;
						(*itr)->addGroundingObject((*opp));//���̃I�u�W�F�N�g�̈ړ��ɁA��ɍڂ��Ă���I�u�W�F�N�g��Ǐ]������
					}
				}

				if ((*opp)->isMovable)//��Ɠ��l
				{
					(*opp)->setPosition((*opp)->getPosition() - collisionVector);
					if (groundCollision(-collisionVector))
					{
						(*itr)->isGrounding = true;
						(*opp)->addGroundingObject((*itr));
					}
				}
			}

			//���݂��̏Փˎ��̏������s��
			collision(*itr, *opp);
		}
	}

	//�v���C���[�L�����N�^�[�ɂ��Ă̓����蔻����s��
	for (auto itr = sceneModels.begin(); itr != sceneModels.end(); itr++)
	{
		if ((*itr)->getColider())
		{
			if (player->getColider()->Intersect((*itr)->getColider(), collisionVector))
			{
				if ((*itr)->isMovable)
				{
					(*itr)->setPosition((*itr)->getPosition() + collisionVector);
				}

				if (player->isMovable)
				{
					if (groundCollision(collisionVector))
					{
						player->isGrounding = true;
						(*itr)->addGroundingObject(player);
					}

					player->setPosition(player->getPosition() - collisionVector);
				}

				collision(player, (*itr));
			}
		}
	}
}

//�I�u�W�F�N�g�̏Փˎ��̏���
void Scene::collision(std::weak_ptr<Model> model,std::weak_ptr<Model> model2)
{
	//���ꂼ��̃I�u�W�F�N�g�̏Փˏ���
	model.lock()->getThisWeakPtr().lock()->collision(model2.lock()->getThisWeakPtr());

	model2.lock()->getThisWeakPtr().lock()->collision(model.lock()->getThisWeakPtr());
}

//�t���[���I�����Ɏ��s
void Scene::frameEnd()
{
	//���C�g�̃��j�t�H�[���o�b�t�@�̍X�V�͕ʘg�ōs��
	updateLightUniformBuffer();

	//�����蔻��̏����ɂ��A�ړ������I�u�W�F�N�g�̍��W�ϊ��s���R���C�_�[�̈ʒu���X�V����
	//���j�t�H�[���o�b�t�@�̍X�V
	for (auto itr = sceneModels.begin(); itr != sceneModels.end();)
	{
		(*itr)->frameEnd(sceneDirectionalLights, scenePointLights, shadowMapData);

		if (!(*itr)->isExist())
		{
			//�I�u�W�F�N�g���폜����
			itr = sceneModels.erase(itr);

			continue;
		}

		itr++;
	}

	player->frameEnd(sceneDirectionalLights,scenePointLights,shadowMapData);
	cubemap->frameEnd();

	for (auto itr = sceneUI.begin(); itr != sceneUI.end();)
	{
		(*itr)->frameEnd();

		if (!(*itr)->isExist())
		{
			//UI���폜����
			itr = sceneUI.erase(itr);

			continue;
		}

		itr++;
	}
}

//���C�g�ƃV���h�E�}�b�v�̃��j�t�H�[���o�b�t�@�̍X�V
void Scene::updateLightUniformBuffer()
{
	//�f�B���N�V���i�����C�g�̍X�V
	updateDirLightUniformBuffer();
	//�|�C���g���C�g�̍X�V
	updatePointLightUniformBuffer();
	//�V���h�E�}�b�v�̍X�V
	updateShadowMapUniformBuffer();
}

//�f�B���N�V���i�����C�g�̍X�V
void Scene::updateDirLightUniformBuffer()
{
	if (sceneDirectionalLights.size() == 0)
	{
		return;
	}

	DirectionalLightUBO ubo{};

	int loopLimit = static_cast<int>(ubo.dir.size());
	if (loopLimit > static_cast<int>(sceneDirectionalLights.size()))
	{
		loopLimit = static_cast<int>(sceneDirectionalLights.size());
	}

	ubo.lightCount = loopLimit;

	{
		int index = 0;
		for (auto itr = sceneDirectionalLights.begin(); itr != sceneDirectionalLights.end(); itr++)
		{
			ubo.dir[index] = glm::vec4((*itr)->direction, 0.0f);
			ubo.color[index] = glm::vec4((*itr)->color, 0.0f);

			if (index >= loopLimit)
			{
				break;
			}

			index++;
		}
	}

	memcpy(dirLightBuffer.mappedBuffer.uniformBufferMapped, &ubo, sizeof(DirectionalLightUBO));
}

//�|�C���g���C�g�̍X�V
void Scene::updatePointLightUniformBuffer()
{
	if (scenePointLights.size() == 0)
	{
		return;
	}

	PointLightUBO ubo{};

	int loopLimit = static_cast<int>(std::min(ubo.color.size(), scenePointLights.size()));

	ubo.lightCount = loopLimit;

	{
		int index = 0;
		for (auto itr = scenePointLights.begin();itr != scenePointLights.end();itr++)
		{
			ubo.pos[index] = glm::vec4((*itr)->getPosition(), 1.0f);
			ubo.color[index] = glm::vec4((*itr)->color, 1.0f);

			if (index >= loopLimit)
			{
				break;
			}

			index++;
		}
	}

	memcpy(pointLightBuffer.mappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
}

//�V���h�E�}�b�v�̃��j�t�H�[���o�b�t�@�̍X�V
void Scene::updateShadowMapUniformBuffer()
{
	Storage* storage = Storage::GetInstance();

	std::shared_ptr<Camera> camera = storage->accessCamera();

	{
		int index = 0;
		for (auto itr = sceneDirectionalLights.begin();itr != sceneDirectionalLights.end();itr++)
		{
			shadowMapData.matUBOs[index].view = glm::lookAt((*itr)->getPosition()
				, (*itr)->getPosition() + (*itr)->direction, glm::vec3(0.0f, 1.0f, 0.0f));

			shadowMapData.matUBOs[index].proj = shadowMapData.proj;

			memcpy(shadowMapData.mappedBuffers[index].uniformBufferMapped, &shadowMapData.matUBOs[index], sizeof(ShadowMapUBO));

			index++;
		}
	}
}

//gpu��̃o�b�t�@��j������
//�V�[���Ǌ��̃C���X�^���X�̃o�b�t�@�̂ݔj������
void Scene::cleanupVulkan()
{
	Storage* storage = Storage::GetInstance();

	VulkanBase* vulkan = VulkanBase::GetInstance();
	VkDevice device = vulkan->getDevice();

	//�������񏈗����I���܂őҋ@����
	vulkan->gpuWaitIdle();

	shadowMapData.destroy(device);

	//���C�g
	pointLightBuffer.mappedBuffer.destroy(device);
	dirLightBuffer.mappedBuffer.destroy(device);
}

void Scene::render()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	//�����_�����O�̊J�n
	vulkan->renderBegin();

	//�܂��̓V���h�E�}�b�v�̍쐬
	vulkan->shadowMapBegin(shadowMapData);

	for (auto model : sceneModels)
	{
		//�������̃I�u�W�F�N�g�̉e�͍��Ȃ�
		if (!model->isTransparent() || true)
		{
			vulkan->renderShadowMap(model, shadowMapData);
		}
	}
	vulkan->renderShadowMap(player, shadowMapData);

	//�V���h�E�}�b�v�̍쐻�I��
	vulkan->shadowMapEnd();

	//3D���f����UI�̃����_�����O�J�n
	vulkan->sceneRenderBegin();

	//�������̃I�u�W�F�N�g�̃����_�����O�͌�ɍs��

	//UI
	for (auto ui : sceneUI)
	{
		if (!ui->isTransparent())
		{
			vulkan->renderUI(ui);
		}
	}

	//3D���f��
	for (auto model : sceneModels)
	{
		if (!model->isTransparent())
		{
			vulkan->renderModel(model, cubemap
				, shadowMapData, pointLightBuffer, dirLightBuffer);
		}
	}

	//�L���[�u�}�b�v
	vulkan->renderCubemap(cubemap);

	//�������̃I�u�W�F�N�g�������_�����O����

	//3D���f��
	for (auto model : sceneModels)
	{
		if (model->isTransparent())
		{
			vulkan->renderModel(model, cubemap
				, shadowMapData, pointLightBuffer, dirLightBuffer);
		}
	}
	vulkan->renderModel(player, cubemap, shadowMapData, pointLightBuffer, dirLightBuffer);

	//UI
	for (auto ui : sceneUI)
	{
		if (ui->isTransparent())
		{
			vulkan->renderUI(ui);
		}
	}

	//3D���f����UI�̃����_�����O�I��
	vulkan->sceneRenderEnd();

	vulkan->renderEnd();
}