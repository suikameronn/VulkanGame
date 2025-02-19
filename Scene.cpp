#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)//lua�t�@�C���̃p�X���󂯎��
{

	camera = std::make_shared<Camera>();

	initLuaScript(luaScriptPath);//lua����X�e�[�W�̃f�[�^��ǂݎ��

	initFrameSetting();//lua����ǂݎ�����I�u�W�F�N�g�̏���������
}

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::initFrameSetting()
{
	for (int i = 0; i < sceneModels.size(); i++)//���ׂẴI�u�W�F�N�g�̏���������
	{
		sceneModels[i]->initFrameSetting();//�I�u�W�F�N�g�̏���������
	}
	player->initFrameSetting();//�v���C���[�N���X�̂ݕʘg

	setLights();//���C�g�̑��̃N���X�̃f�[�^��p��
	setModels();//���f���̑��̃N���X�̃f�[�^��p��
}

void Scene::initLuaScript(std::string path)//lua�X�N���v�g�̓ǂݎ��
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);
	lua_setglobal(lua, "Scene");//�C���X�^���X��lua�X�N���v�g�Ŏg����悤�ɂ���

	registerOBJECT();
	registerFunctions();

	luaL_dofile(lua,path.c_str());
}

void Scene::registerOBJECT()//enum���X�N���v�g�Ƌ��L����
{
	lua_pushnumber(lua, (int)GLTFOBJECT::gltfTEST);
	lua_setglobal(lua, "gltfTEST");//�L�����N�^�[

	lua_pushnumber(lua, (int)GLTFOBJECT::ASPHALT);
	lua_setglobal(lua, "ASPHALT");//�A�X�t�@���g

	lua_pushnumber(lua, (int)GLTFOBJECT::LEATHER);
	lua_setglobal(lua, "LEATHER");//�����v
}

void Scene::registerFunctions()//lua�Ɋ֐���o�^
{
	lua_register(lua, "glueCreateModel", glueCreateModel);//3D���f���N���X�̒ǉ�
	lua_register(lua, "glueCreatePlayer", glueCreatePlayer);//�v���C���[�̒ǉ�
	lua_register(lua, "glueSetLuaPath", glueSetLuaPath);//�X�N���v�g���I�u�W�F�N�g�ɒǉ�
	lua_register(lua, "glueSetGltfModel",glueSetGltfModel);//gltf���f���̒ǉ�
	lua_register(lua, "glueSetPos", glueSetPos);//���W�̐ݒ�
	lua_register(lua, "glueSetRotate", glueSetRotate);//������ݒ�
	lua_register(lua, "glueSetScale", glueSetScale);//���f���̃X�P�[���̐ݒ�
	lua_register(lua, "glueSetBaseColor", glueSetBaseColor);//Diffuse�J���[�̐ݒ�
	lua_register(lua, "glueBindCamera", glueBindCamera);//�J�������Ǐ]����I�u�W�F�N�g��ݒ肷��
	lua_register(lua, "glueSetColider", glueSetColider);//���f���ɃR���C�_�[��t����
	lua_register(lua, "glueSetColiderScale", glueSetColiderScale);//�R���C�_�[�̃X�P�[����ݒ�
	lua_register(lua, "glueSetDefaultAnimationName", glueSetDefaultAnimationName);//���f���Ƀf�t�H���g�̃A�j���[�V������ݒ肷��
	lua_register(lua, "glueSetGravity", glueSetGravity);//�I�u�W�F�N�g�ɏd�ʂ���������
	lua_register(lua, "glueSetSlippery", glueSetSlippery);//���C�͂�ݒ肷��
	lua_register(lua, "glueCreatePointLight", glueCreatePointLight);//�|�C���g���C�g���쐬
	lua_register(lua, "glueSetLightColor", glueSetLightColor);//���C�g�̃J���[��ݒ�
	lua_register(lua, "glueCreateDirectionalLight", glueCreateDirectionalLight);//���s�������쐬
	lua_register(lua, "glueSetLightDirection", glueSetLightDirection);//���s�����̕�����ݒ�
	lua_register(lua, "glueBindObject", glueBindObject);//�I�u�W�F�N�g�ɐe�q�֌W��ݒ肷��
}

bool Scene::UpdateScene()//�V�[���S�̂̃A�b�v�f�[�g����
{
	bool exit = false;

	camera->Update();

	player->Update();

	for (int i = 0; i < sceneModels.size(); i++)
	{
		sceneModels[i]->Update();

		if (sceneModels[i]->uniformBufferChange)//�I�u�W�F�N�g�̈ʒu��������ς�����ꍇ
		{
			sceneModels[i]->updateTransformMatrix();//MVP�̃��f���s����X�V����
		}
	}

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
					}

					if (sceneModels[j]->isMovable)
					{
						sceneModels[j]->setPosition(sceneModels[j]->getPosition() - collisionVector);
					}
				}
			}
		}
	}

	//Player�N���X�ƃ��f���N���X�̓����蔻��
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
					player->cancelGravity();
				}

				player->setPosition(player->getPosition() - collisionVector);
			}
		}
	}

	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (sceneModels[i]->uniformBufferChange)//�Փˉ����ɂ���ă��f�����ړ�������A�ēx�s��̍X�V
		{
			sceneModels[i]->updateTransformMatrix();
		}
	}
	player->updateTransformMatrix();

	return exit;
}

void Scene::setModels()
{
	Storage::GetInstance()->setCamera(camera);

	//�`�悷�郂�f���̃|�C���^��ς�ł���
	for (int i = 0;i < sceneModels.size();i++)
	{
		sceneModels[i]->updateTransformMatrix();
		Storage::GetInstance()->addModel(sceneModels[i]);
	}

	player->updateTransformMatrix();
	Storage::GetInstance()->addModel(player);
}

void Scene::setLights()
{
	for (std::shared_ptr<PointLight> pl : scenePointLights)
	{
		pl->updateTransformMatrix();
		Storage::GetInstance()->addLight(pl);
	}

	for (std::shared_ptr<DirectionalLight> dl : sceneDirectionalLights)
	{
		dl->updateTransformMatrix();
		Storage::GetInstance()->addLight(dl);
	}

	//Light��Vulkan�ł̕ϐ��Ȃǂ���������
	Storage::GetInstance()->prepareLightsForVulkan();
}

bool Scene::groundCollision(glm::vec3 collisionVector)//�����𒲐�����
{
	glm::vec3 v = -glm::normalize(collisionVector);

	float dot = glm::dot(v, up);

	return dot > 0;
}

std::shared_ptr<Model> Scene::raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model)//�������΂��āA�R���C�_�[��T��
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
		if (colider2->Intersect(origin,dir,length))//�R���C�_�[�Ɛ����̏Փ˔���
		{
			return sceneModels[i];
		}
	}

	return nullptr;
}