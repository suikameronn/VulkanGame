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

	for (int i = 0; i < sceneModels.size(); i++)//���ׂẴI�u�W�F�N�g�̏���������
	{
		vulkan->setModelData(sceneModels[i]);
		sceneModels[i]->initFrameSetting();//�I�u�W�F�N�g�̏���������
	}

	vulkan->setModelData(player);
	player->initFrameSetting();//�v���C���[�N���X�̂ݕʘg
	player->setPosition(startPoint);//�v���C���[�������ʒu��

	vulkan->createCubemap(cubemap);

	for (int i = 0; i < sceneUI.size(); i++)
	{
		vulkan->setUI(sceneUI[i]);
		sceneUI[i]->initFrameSettings();
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
}

void Scene::registerFunctions()//lua����Ăяo�����ÓI�֐���ݒ�
{
	lua_register(lua, "createModel", glueSceneFunction::glueCreateModel);//3D���f���N���X�̒ǉ�
	lua_register(lua, "createPlayer", glueSceneFunction::glueCreatePlayer);//�v���C���[�̒ǉ�
	lua_register(lua, "setLuaPath", glueSceneFunction::glueSetLuaPath);//�X�N���v�g���I�u�W�F�N�g�ɒǉ�
	lua_register(lua, "setDelayStartLua", glueSceneFunction::glueSetDelayStartLua);//�X�N���v�g�̎��s�J�n�^�C�~���O��ݒ�
	lua_register(lua, "setGltfModel", glueSceneFunction::glueSetGltfModel);//gltf���f���̒ǉ�
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

	for (int i = 0; i < sceneModels.size(); i++)//���ׂẴI�u�W�F�N�g�̍X�V����
	{
		sceneModels[i]->Update();//�I�u�W�F�N�g�̍X�V����

		if (sceneModels[i]->uniformBufferChange)//�I�u�W�F�N�g�̈ʒu��������ς�����ꍇ
		{
			sceneModels[i]->updateTransformMatrix();//MVP�̃��f���s����X�V����
		}
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
std::shared_ptr<Model> Scene::raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model,glm::vec3& normal)
{
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (model == sceneModels[i].get())
		{
			continue;
		}

		if (!sceneModels[i]->hasColider())
		{
			continue;
		}

		std::shared_ptr<Colider> colider2 = sceneModels[i]->getColider();
		if (colider2->Intersect(origin,dir,length,normal))//�R���C�_�[�Ɛ����̏Փ˔���
		{
			return sceneModels[i];
		}
	}

	return nullptr;
}

//�V�[���S�̂�R�؂ɃI�u�W�F�N�g��ǉ�����
void Scene::addModelToRTree(Model* model)
{
	rtree->insert(model, model->getMbrMin(), model->getMbrMax());
}

//�c���[���̃I�u�W�F�N�g�̈ʒu���X�V����
void Scene::updateObjectPos(Model* model, RNode<Model>* node)
{
	rtree->reflectMove(model, node);
}

void Scene::rtreeIntersect()
{
	for (int i = 0; i < sceneModels.size(); i++)//���f�����m�̓����蔻����s��
	{
		if (!sceneModels[i]->hasColider())//�R���C�_�[�������Ă��Ȃ�������X�L�b�v
		{
			continue;
		}

		//R�؂��g���āA�����蔻����s���I�u�W�F�N�g���i��
		std::vector<Model*> collisionDetectTarget;
		rtree->broadPhaseCollisionDetection(collisionDetectTarget, sceneModels[i]->getMbrMin(), sceneModels[i]->getMbrMax());

		for (int j = 0; j < collisionDetectTarget.size(); j++)
		{
			//�R���C�_�[���ݒ肳��Ă��邩�A�������g�������I�u�W�F�N�g�Ɠ����蔻����s��
			if (collisionDetectTarget[j]->hasColider() && sceneModels[i].get() == collisionDetectTarget[j])
			{
				if (sceneModels[i]->getColider()->Intersect(collisionDetectTarget[j]->getColider(), collisionVector))//GJK�ɂ�铖���蔻����s��
				{
					if (sceneModels[i]->isMovable)//�ǂȂǓ������Ȃ����̂͏��O����
					{
						sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);//�Փ˂���������
						if (groundCollision(collisionVector))//������ڂ̃I�u�W�F�N�g����߂̃I�u�W�F�N�g�̐^��ɂ���
							//�܂�A���̂悤�ȃI�u�W�F�N�g�̏�ɍڂ��Ă���󋵂̏ꍇ
						{
							collisionDetectTarget[j]->isGrounding = true;
							sceneModels[i]->addGroundingObject(collisionDetectTarget[j]);//���̃I�u�W�F�N�g�̈ړ��ɁA��ɍڂ��Ă���I�u�W�F�N�g��Ǐ]������
						}
					}

					if (collisionDetectTarget[j]->isMovable)//��Ɠ��l
					{
						collisionDetectTarget[j]->setPosition(collisionDetectTarget[j]->getPosition() - collisionVector);
						if (groundCollision(-collisionVector))
						{
							sceneModels[i]->isGrounding = true;
							collisionDetectTarget[j]->addGroundingObject(sceneModels[i].get());
						}
					}
				}
			}
		}
	}

	{//�v���C���[�L�����N�^�[�ɂ��Ă̓����蔻����s��
		std::vector<Model*> collisionDetectTarget;
		rtree->broadPhaseCollisionDetection(collisionDetectTarget, player->getMbrMin(), player->getMbrMax());

		for (int i = 0; i < collisionDetectTarget.size(); i++)
		{
			if (collisionDetectTarget[i]->getColider())
			{
				if (player->getColider()->Intersect(collisionDetectTarget[i]->getColider(), collisionVector))
				{
					if (collisionDetectTarget[i]->isMovable)
					{
						collisionDetectTarget[i]->setPosition(collisionDetectTarget[i]->getPosition() + collisionVector);
					}

					if (player->isMovable)
					{
						if (groundCollision(collisionVector))
						{
							player->isGrounding = true;
							collisionDetectTarget[i]->addGroundingObject(player.get());
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
	for (int i = 0; i < sceneModels.size() - 1; i++)//���f�����m�̓����蔻����s��
	{
		if (!sceneModels[i]->hasColider())//�R���C�_�[�������Ă��Ȃ�������X�L�b�v
		{
			continue;
		}

		for (int j = i + 1; j < sceneModels.size(); j++)
		{
			if (!sceneModels[j]->hasColider())
			{
				continue;
			}

			//�R���C�_�[���ݒ肳��Ă��邩�A�������g�������I�u�W�F�N�g�Ɠ����蔻����s��
			if (sceneModels[i]->getColider()->Intersect(sceneModels[j]->getColider(), collisionVector))//GJK�ɂ�铖���蔻����s��
			{
				if (sceneModels[i]->isMovable)//�ǂȂǓ������Ȃ����̂͏��O����
				{
					sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);//�Փ˂���������
					if (groundCollision(collisionVector))//������ڂ̃I�u�W�F�N�g����߂̃I�u�W�F�N�g�̐^��ɂ���
						//�܂�A���̂悤�ȃI�u�W�F�N�g�̏�ɍڂ��Ă���󋵂̏ꍇ
					{
						sceneModels[j]->isGrounding = true;
						sceneModels[i]->addGroundingObject(sceneModels[j].get());//���̃I�u�W�F�N�g�̈ړ��ɁA��ɍڂ��Ă���I�u�W�F�N�g��Ǐ]������
					}
				}

				if (sceneModels[j]->isMovable)//��Ɠ��l
				{
					sceneModels[j]->setPosition(sceneModels[j]->getPosition() - collisionVector);
					if (groundCollision(-collisionVector))
					{
						sceneModels[i]->isGrounding = true;
						sceneModels[j]->addGroundingObject(sceneModels[i].get());
					}
				}
			}
		}
	}

	//�v���C���[�L�����N�^�[�ɂ��Ă̓����蔻����s��
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (sceneModels[i]->getColider())
		{
			if (player->getColider()->Intersect(sceneModels[i]->getColider(), collisionVector))
			{
				if (sceneModels[i]->isMovable)
				{
					sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);
				}

				if (player->isMovable)
				{
					if (groundCollision(collisionVector))
					{
						player->isGrounding = true;
						sceneModels[i]->addGroundingObject(player.get());
					}

					player->setPosition(player->getPosition() - collisionVector);
				}
			}
		}
	}
}

//�t���[���I�����Ɏ��s
void Scene::frameEnd()
{
	//���C�g�̃��j�t�H�[���o�b�t�@�̍X�V�͕ʘg�ōs��
	updateLightUniformBuffer();

	//�����蔻��̏����ɂ��A�ړ������I�u�W�F�N�g�̍��W�ϊ��s���R���C�_�[�̈ʒu���X�V����
	//���j�t�H�[���o�b�t�@�̍X�V
	for (auto model : sceneModels)
	{
		model->frameEnd(sceneDirectionalLights, scenePointLights, shadowMapData);
	}
	player->frameEnd(sceneDirectionalLights,scenePointLights,shadowMapData);
	cubemap->frameEnd();

	for (auto ui : sceneUI)
	{
		ui->frameEnd();
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

	for (int i = 0; i < loopLimit; i++)
	{
		ubo.dir[i] = glm::vec4(sceneDirectionalLights[i]->direction, 0.0f);
		ubo.color[i] = glm::vec4(sceneDirectionalLights[i]->color, 0.0f);
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

	for (int i = 0; i < loopLimit; i++)
	{
		ubo.pos[i] = glm::vec4(scenePointLights[i]->getPosition(), 1.0f);
		ubo.color[i] = glm::vec4(scenePointLights[i]->color, 1.0f);
	}

	memcpy(pointLightBuffer.mappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
}

//�V���h�E�}�b�v�̃��j�t�H�[���o�b�t�@�̍X�V
void Scene::updateShadowMapUniformBuffer()
{
	Storage* storage = Storage::GetInstance();

	std::shared_ptr<Camera> camera = storage->accessCamera();

	for (int i = 0; i < sceneDirectionalLights.size(); i++)
	{
		shadowMapData.matUBOs[i].view = glm::lookAt(sceneDirectionalLights[i]->getPosition()
			, sceneDirectionalLights[i]->getPosition() + sceneDirectionalLights[i]->direction, glm::vec3(0.0f, 1.0f, 0.0f));

		shadowMapData.matUBOs[i].proj = shadowMapData.proj;

		memcpy(shadowMapData.mappedBuffers[i].uniformBufferMapped, &shadowMapData.matUBOs[i], sizeof(ShadowMapUBO));
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

	//���f��
	for (auto model : sceneModels)
	{
		model->cleanupVulkan();
	}
	player->cleanupVulkan();

	cubemap->cleanupVulkan();

	//UI
	for (auto ui : sceneUI)
	{
		ui->cleanupVulkan();
	}

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

	//�������̃I�u�W�F�N�g�������_�����O����

	//UI
	for (auto ui : sceneUI)
	{
		if (ui->isTransparent())
		{
			vulkan->renderUI(ui);
		}
	}

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

	//�L���[�u�}�b�v
	vulkan->renderCubemap(cubemap);

	//3D���f����UI�̃����_�����O�I��
	vulkan->sceneRenderEnd();

	vulkan->renderEnd();
}