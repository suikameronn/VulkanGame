#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)//lua�t�@�C���̃p�X���󂯎��
{
	startPoint = glm::vec3(0.0f);

	camera = std::make_shared<Camera>();
	Storage::GetInstance()->setCamera(camera);

	rtree = std::make_unique<RTree>();

	initLuaScript(luaScriptPath);//lua����X�e�[�W�̃f�[�^��ǂݎ��

	initFrameSetting();//lua����ǂݎ�����I�u�W�F�N�g�̏���������
}

Scene::Scene()
{
}

Scene::~Scene()
{
}

//����t���[���̂ݎ��s �X�e�[�W��̂��ׂẴI�u�W�F�N�g�̏���t���[�����̐ݒ���s��
void Scene::initFrameSetting()
{
	for (int i = 0; i < sceneModels.size(); i++)//���ׂẴI�u�W�F�N�g�̏���������
	{
		sceneModels[i]->initFrameSetting();//�I�u�W�F�N�g�̏���������
	}
	player->initFrameSetting();//�v���C���[�N���X�̂ݕʘg
	player->setPosition(startPoint);//�v���C���[�������ʒu��

	Storage::GetInstance()->prepareDescriptorSets();

	setLights();//���C�g�̑��̃N���X�̃f�[�^��p��

	Storage::GetInstance()->prepareLightsForVulkan();//Light��Vulkan�ł̕ϐ��Ȃǂ���������
	Storage::GetInstance()->prepareDescriptorData();//descriptorSet�̗p��

	setModels();//���f���̑��̃N���X�̃f�[�^��p��
}

//lua�X�N���v�g�̓ǂݎ��A���s
void Scene::initLuaScript(std::string path)
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);//����Scene�̃C���X�^���X��lua�̉��z�}�V���ɑ���
	lua_setglobal(lua, "Scene");//�C���X�^���X��lua�X�N���v�g�Ŏg����悤�ɂ���

	registerOBJECT();//gltf���f�����w�肵�邽�߂̔ԍ��𑗂�
	registerFunctions();//�X�N���v�g����Ăяo���֐���o�^����

	luaL_dofile(lua,path.c_str());//�X�N���v�g�̎��s
}

//lua�̉��z�}�V����gltf���f�����w�肷�邽�߂̔ԍ���ݒ肷��
void Scene::registerOBJECT()//enum���X�N���v�g�Ƌ��L����
{
	lua_pushnumber(lua, (int)GLTFOBJECT::gltfTEST);
	lua_setglobal(lua, "gltfTEST");//�L�����N�^�[

	lua_pushnumber(lua, (int)GLTFOBJECT::ASPHALT);
	lua_setglobal(lua, "ASPHALT");//�A�X�t�@���g

	lua_pushnumber(lua, (int)GLTFOBJECT::LEATHER);
	lua_setglobal(lua, "LEATHER");//�����v
}

void Scene::registerFunctions()//lua����Ăяo�����ÓI�֐���ݒ�
{
	lua_register(lua, "glueCreateModel", glueSceneFunction::glueCreateModel);//3D���f���N���X�̒ǉ�
	lua_register(lua, "glueCreatePlayer", glueSceneFunction::glueCreatePlayer);//�v���C���[�̒ǉ�
	lua_register(lua, "glueSetLuaPath", glueSceneFunction::glueSetLuaPath);//�X�N���v�g���I�u�W�F�N�g�ɒǉ�
	lua_register(lua, "glueSetDelayStartLua", glueSceneFunction::glueSetDelayStartLua);//�X�N���v�g�̎��s�J�n�^�C�~���O��ݒ�
	lua_register(lua, "glueSetGltfModel", glueSceneFunction::glueSetGltfModel);//gltf���f���̒ǉ�
	lua_register(lua, "glueSetPos", glueSceneFunction::glueSetPos);//���W�̐ݒ�
	lua_register(lua, "glueSetRotate", glueSceneFunction::glueSetRotate);//������ݒ�
	lua_register(lua, "glueSetScale", glueSceneFunction::glueSetScale);//���f���̃X�P�[���̐ݒ�
	lua_register(lua, "glueSetBaseColor", glueSceneFunction::glueSetBaseColor);//Diffuse�J���[�̐ݒ�
	lua_register(lua, "glueBindCamera", glueSceneFunction::glueBindCamera);//�J�������Ǐ]����I�u�W�F�N�g��ݒ肷��
	lua_register(lua, "glueSetAABBColider", glueSceneFunction::glueSetAABBColider);//���f����AABB�R���C�_�[��t����
	lua_register(lua, "glueSetConvexColider", glueSceneFunction::glueSetConvexColider);//���f���ɓʕ�̃R���C�_�[��t����
	lua_register(lua, "glueSetColiderScale", glueSceneFunction::glueSetColiderScale);//�R���C�_�[�̃X�P�[����ݒ�
	lua_register(lua, "glueSetDefaultAnimationName", glueSceneFunction::glueSetDefaultAnimationName);//���f���Ƀf�t�H���g�̃A�j���[�V������ݒ肷��
	lua_register(lua, "glueSetGravity", glueSceneFunction::glueSetGravity);//�I�u�W�F�N�g�ɏd�ʂ���������
	lua_register(lua, "glueCreatePointLight", glueSceneFunction::glueCreatePointLight);//�|�C���g���C�g���쐬
	lua_register(lua, "glueSetLightColor", glueSceneFunction::glueSetLightColor);//���C�g�̃J���[��ݒ�
	lua_register(lua, "glueCreateDirectionalLight", glueSceneFunction::glueCreateDirectionalLight);//���s�������쐬
	lua_register(lua, "glueSetLightDirection", glueSceneFunction::glueSetLightDirection);//���s�����̕�����ݒ�
	lua_register(lua, "glueBindObject", glueSceneFunction::glueBindObject);//�I�u�W�F�N�g�ɐe�q�֌W��ݒ肷��
	lua_register(lua, "glueSetStartPoint", glueSceneFunction::glueSetStartPoint);//�������W�̐ݒ�
	lua_register(lua, "glueSetLimitY", glueSceneFunction::glueSetLimitY);//y���W�̉����̐ݒ�
	lua_register(lua, "glueSetHDRIMap", glueSceneFunction::glueSetHDRIMap);//HDRI�摜�̐ݒ�
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
	hdriMap = FileManager::GetInstance()->loadImage(imagePath);
	Storage::GetInstance()->setCubemapTexture(hdriMap);
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

	/*

	for (int i = 0; i < sceneModels.size() - 1; i++)//���f�����m�̓����蔻����s��
	{
		if (!sceneModels[i]->hasColider())//�R���C�_�[�������Ă��Ȃ�������X�L�b�v
		{
			continue;
		}

		for (int j = i + 1; j < sceneModels.size(); j++)
		{
			if (sceneModels[j]->hasColider())
			{
				if (sceneModels[i]->getColider()->Intersect(sceneModels[j]->getColider(), collisionVector))//GJK�ɂ�铖���蔻����s��
				{
					if (sceneModels[i]->isMovable)//�ǂȂǓ������Ȃ����̂͏��O����
					{
						sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);//�Փ˂���������
						if (groundCollision(collisionVector))//������ڂ̃I�u�W�F�N�g����߂̃I�u�W�F�N�g�̐^��ɂ���
															 //�܂�A���̂悤�ȃI�u�W�F�N�g�̏�ɍڂ��Ă���󋵂̏ꍇ
						{
							sceneModels[j]->isGrounding = true;
							sceneModels[i]->addGroundingObject(sceneModels[j]);//���̃I�u�W�F�N�g�̈ړ��ɁA��ɍڂ��Ă���I�u�W�F�N�g��Ǐ]������
						}
					}

					if (sceneModels[j]->isMovable)//��Ɠ��l
					{
						sceneModels[j]->setPosition(sceneModels[j]->getPosition() - collisionVector);
						if (groundCollision(-collisionVector))
						{
							sceneModels[i]->isGrounding = true;
							sceneModels[j]->addGroundingObject(sceneModels[i]);
						}
					}
				}
			}
		}
	}

	*/

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
			if (collisionDetectTarget[j]->hasColider())
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

	//Player�N���X�ƃ��f���N���X�̓����蔻��
	//���A���Model�N���X���m�̏����Ɠ��l
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (!sceneModels[i]->hasColider())
		{
			continue;
		}

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

	//�����蔻��̏����ɂ��A�ړ������I�u�W�F�N�g�̍��W�ϊ��s���R���C�_�[�̈ʒu���X�V����
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (sceneModels[i]->uniformBufferChange)//�Փˉ����ɂ���ă��f�����ړ�������A�ēx�s��̍X�V
		{
			sceneModels[i]->updateTransformMatrix();
		}
	}
	player->updateTransformMatrix();

	//�����v���C���[���X�e�[�W�̉����𒴂�����A���X�^�[�g������
	if (player->getPosition().y < limitY)
	{
		player->restart(startPoint);
	}

	if (Controller::GetInstance()->getKey(GLFW_KEY_ESCAPE))
	{
		exit = GAME_FINISH;
	}

	return exit;//�E�B���h�E����悤�Ƃ����ꍇ�́Afals�𑗂�A�Q�[���̏I�������������� 
}

//�X�e�[�W��̃I�u�W�F�N�g��Vulkan�̕ϐ���ݒ肷��
void Scene::setModels()
{
	//�`�悷�郂�f���̃|�C���^��ς�ł���
	for (int i = 0;i < sceneModels.size();i++)
	{
		sceneModels[i]->updateTransformMatrix();//���W�ϊ��s��̍X�V
		Storage::GetInstance()->addModel(sceneModels[i]);//gpu��ɒ��_�p�Ȃǂ̃o�b�t�@�[���m�ۂ���
	}

	player->updateTransformMatrix();//���W�ϊ��s��̍X�V
	Storage::GetInstance()->addModel(player);//�v���C���[�̈���3D���f���ɂ����Ă����l�o�b�t�@�[���m�ۂ���
}

//�X�e�[�W��̃��C�g��Vulkan�̕ϐ���ݒ肷��
void Scene::setLights()
{
	Storage* storage = Storage::GetInstance();

	std::shared_ptr<Model> cubemap = std::shared_ptr<Model>(new Model());//�L���[�u�}�b�v�p�̗����̂̏���
	cubemap->setgltfModel(FileManager::GetInstance()->loadModel("models/cubemap.glb"));
	storage->setCubeMapModel(cubemap);

	for (std::shared_ptr<PointLight> pl : scenePointLights)
	{
		pl->updateTransformMatrix();
		storage->addLight(pl);//�o�b�t�@�[�̊m��
	}

	for (std::shared_ptr<DirectionalLight> dl : sceneDirectionalLights)
	{
		dl->updateTransformMatrix();
		storage->addLight(dl);//�o�b�t�@�[�̊m��
	}
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
void Scene::updateObjectPos(Model* model, RNode* node)
{
	rtree->reflectMove(model, node);
}