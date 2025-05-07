#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include "lua/include/lua.hpp"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"

#include"FileManager.h"
#include"Controller.h"
#include"Player.h"
#include"Camera.h"
#include"EnumList.h"
#include"Light.h"
#include"UI.h"
#include"Cubemap.h"

#include<chrono>
#include <thread>

enum
{
	GAME_CONTINUE = 0,
	GAME_FINISH,
	GAME_RESTART
};

//�|�C���g���C�g�̍\���� �����̃��C�g������
struct PointLightUBO
{
	alignas(16)int lightCount;
	alignas(16) std::array<glm::vec4, 50> pos;
	alignas(16) std::array<glm::vec4, 50> color;
};

//���s�����̍\���� �����̃��C�g������
struct DirectionalLightUBO
{
	alignas(16) int lightCount;
	alignas(16) std::array<glm::vec4, 50> dir;
	alignas(16) std::array<glm::vec4, 50> color;
};

//�V���h�E�}�b�v�쐬�p��uniform buffer
struct ShadowMapUBO
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct DirectionalLightBuffer
{
	DirectionalLightUBO ubo;
	MappedBuffer mappedBuffer;
	VkDescriptorSet descriptorSet;
};

struct PointLightBuffer
{
	PointLightUBO ubo;
	MappedBuffer mappedBuffer;
	VkDescriptorSet descriptorSet;
};

//�V���h�E�}�b�v�쐬�p�̍\����
struct ShadowMapData
{
	//���t���e�̍s��
	glm::mat4 proj;

	//�V���h�E�}�b�v�̉𑜓x�̔{����ݒ�
	int shadowMapScale;
	//�I�t�X�N���[�������_�����O�p�̍\����
	OffScreenPass passData;
	//�V���h�E�}�b�v�쐬�p�̍s��̔z��
	std::vector<ShadowMapUBO> matUBOs;
	//�s��p�̃o�b�t�@�̔z��
	std::vector<MappedBuffer> mappedBuffers;

	//�V���h�E�}�b�v��ʏ�̃����_�����O�Ŏg�p���邽�߂̃f�[�^
	std::vector<VkDescriptorSet> descriptorSets;

	//�V�[����̃��C�g�̐������쐬
	void setFrameCount(int frameCount)
	{
		matUBOs.resize(frameCount);
		mappedBuffers.resize(frameCount);
		passData.setFrameCount(frameCount);
	}

	void destroy(VkDevice& device)
	{
		passData.destroy(device);

		for (auto& buffer : mappedBuffers)
		{
			buffer.destroy(device);
		}
	}
};


//lua����ǂݎ�����X�e�[�W�̏��Ȃǂ��i�[�����
class Scene
{
private:

	//�V�[���S�̂̃|�C���g���C�g�p�̃o�b�t�@
	PointLightBuffer pointLightBuffer;
	//�V�[���S�̂̃f�B���N�V���i�����C�g�p�̃o�b�t�@
	DirectionalLightBuffer dirLightBuffer;
	//�V���h�E�}�b�v�p�o�b�t�@
	ShadowMapData shadowMapData;

	static Scene* instance;

	std::unique_ptr<RTree<Model>> rtree;

	//������̃x�N�g��
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 startPoint;//�������W
	float limitY;//y���W�̉��� ����������ƃ��X�^�[�g����

	//����t���[���̂ݎ��s �X�e�[�W��̂��ׂẴI�u�W�F�N�g�̏���t���[�����̐ݒ���s��
	void initFrameSetting();

	//�t���[���I�����Ɏ��s
	void frameEnd();

	float collisionDepth;//�Փˎ��̂߂荞�񂾋���
	glm::vec3 collisionVector;//�Փˎ��̂߂荞�񂾕���

	//lua�̉��z�}�V���ɃA�N�Z�X���邽�߂̕ϐ�
	lua_State* lua;
	//lua�X�N���v�g�̓ǂݎ��A���s
	void initLuaScript(std::string path);
	//lua�̉��z�}�V����gltf���f�����w�肷�邽�߂̔ԍ���ݒ肷��
	void registerOBJECT();
	//lua����Ăяo�����ÓI�֐���ݒ�
	void registerFunctions();

	//�R���C�_�[�����ɐڒn���Ă��邩�̔��� true�̏ꍇ�͂��̏��̈ړ���Ǐ]����
	bool groundCollision(glm::vec3 collisionVector);
	//�I�u�W�F�N�g�̐ڒn����Ȃǂ����Z�b�g
	void resetStatus();

	//R�؂��g���������蔻��
	void rtreeIntersect();
	//�V���v���ȓ����蔻��
	void intersect();

	//���C�g�̃��j�t�H�[���o�b�t�@�̍X�V
	void updateLightUniformBuffer();
	//�f�B���N�V���i�����C�g�̍X�V
	void updateDirLightUniformBuffer();
	//�|�C���g���C�g�̍X�V
	void updatePointLightUniformBuffer();
	//�V���h�E�}�b�v�̍X�V
	void updateShadowMapUniformBuffer();

	Scene();
	~Scene();

	//�����_�����O
	void render();

	//gpu��̃o�b�t�@��j������
	//�V�[���Ǌ��̃C���X�^���X�̃o�b�t�@�̂ݔj������
	void cleanupVulkan();

public:

	static Scene* GetInstance()
	{
		if (!instance)
		{
			instance = new Scene();
		}

		return instance;
	}

	void Destroy()
	{
		delete instance;
		instance = nullptr;
	}

	//�J����
	std::shared_ptr<Camera> camera;

	//lua�X�N���v�g�����s���A�o�^���ꂽ�I�u�W�F�N�g�Ȃǂ̏����t���[�������܂ōs��
	void init(std::string luaScriptPath);

	void setStartPoint(glm::vec3 point);//�������W�̐ݒ�
	void setLimitY(float y);//y���W�̉����̐ݒ�
	
	//�v���C���[
	std::shared_ptr<Player> player;
	//�L���[�u�}�b�s���O�p�̗����̃I�u�W�F�N�g
	std::shared_ptr<Cubemap> cubemap;
	//�X�e�[�W��̃I�u�W�F�N�g�̔z��
	std::vector<std::shared_ptr<Model>> sceneModels;
	//�X�e�[�W��̃|�C���g���C�g�̔z��
	std::vector<std::shared_ptr<PointLight>> scenePointLights;
	//�X�e�[�W��̕��s�����̔z��
	std::vector<std::shared_ptr<DirectionalLight>> sceneDirectionalLights;
	//�X�e�[�W���UI
	std::vector<std::shared_ptr<UI>> sceneUI;

	//�X�e�[�W��̃I�u�W�F�N�g�Ȃǂ̍X�V����
	int UpdateScene();

	//�l�p�`�����΂��āA�Փ˔�����s���A�ڒn����Ɏg����
	std::shared_ptr<Model> raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model,glm::vec3& normal);

	//HDRI�}�b�v�̐ݒ�
	void setHDRIMap(std::string imagePath);

	//�V�[���S�̂̃I�u�W�F�N�g�ɂ��Ă�RTree�ɃI�u�W�F�N�g��ǉ�����
	void addModelToRTree(Model* model);

	//R�c���[���̃I�u�W�F�N�g�̈ʒu���X�V����
	void updateObjectPos(Model* model, RNode<Model>* node);
};

/*�ȉ��̊֐���lua�X�N���v�g����Ăяo�����*/

// �X�^�b�N�̓��e��\������֐� �f�o�b�O�p
static void printStack(lua_State* L) {
	int top = lua_gettop(L); // �X�^�b�N�̃g�b�v�C���f�b�N�X���擾
	
	std::cout << "Stack size: " << top << std::endl;
	for (int i = 1; i <= top; ++i) {
		int t = lua_type(L, i);
		switch (t) {
		case LUA_TSTRING: // ������̏ꍇ
			std::cout << i << ": " << lua_tostring(L, i) << " (string)" << std::endl;
			break;
		case LUA_TBOOLEAN: // �^�U�l�̏ꍇ
			std::cout << i << ": " << (lua_toboolean(L, i) ? "true" : "false") << " (boolean)" << std::endl;
			break;
		case LUA_TNUMBER: // ���l�̏ꍇ
			std::cout << i << ": " << lua_tonumber(L, i) << " (number)" << std::endl;
			break;
		case LUA_TUSERDATA: // ���[�U�[�f�[�^�̏ꍇ
			std::cout << i << ": " << lua_touserdata(L, i) << " (userdata)" << std::endl;
			break;
		case LUA_TTABLE: // �e�[�u���̏ꍇ
			std::cout << i << ": table" << std::endl;
			break;
		default: // ���̑��̏ꍇ
			std::cout << i << ": " << lua_typename(L, t) << std::endl;
			break;
		}
	}
}

namespace glueSceneFunction//Scene�N���X�̗p��glue�֐�
{

	//�I�u�W�F�N�g���쐬���� �ʏ�͎g��Ȃ�
	static int glueCreateObject(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		std::shared_ptr<Object> obj = std::shared_ptr<Object>(new Object());

		return 1;
	}

	//3D���f�������I�u�W�F�N�g���쐬����
	static int glueCreateModel(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		Model* model = new Model();

		lua_pushlightuserdata(lua, model);

		std::shared_ptr<Model> sharedModel = std::shared_ptr<Model>(model);
		scene->sceneModels.push_back(sharedModel);

		return 1;
	}

	//�|�C���g���C�g�̍쐬
	static int glueCreatePointLight(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		PointLight* pointLight = new PointLight();
		scene->scenePointLights.push_back(std::shared_ptr<PointLight>(pointLight));

		lua_pushlightuserdata(lua, pointLight);

		return 1;
	}

	//���s�����̍쐬
	static int glueCreateDirectionalLight(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		DirectionalLight* directionalLight = new DirectionalLight();
		scene->sceneDirectionalLights.push_back(std::shared_ptr<DirectionalLight>(directionalLight));

		lua_pushlightuserdata(lua, directionalLight);

		return 1;
	}

	//�v���C���[�̍쐬 ���łɍ쐬����Ă����ꍇ�͉������Ȃ�
	static int glueCreatePlayer(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		if (scene->player)
		{
			return -1;
		}

		Player* player = new Player();
		scene->player = std::shared_ptr<Player>(player);

		lua_pushlightuserdata(lua, player);

		return 1;
	}

	//�I�u�W�F�N�g�ɍs���p�^�[���p��lua�X�N���v�g��ݒ肷��
	static int glueSetLuaPath(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		std::string path = std::string(lua_tostring(lua, -1));

		obj->setLuaScript(path);

		return 0;
	}

	//lua�X�N���v�g�����s���n�߂�t���[����x��������
	static int glueSetDelayStartLua(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		int delayFrame = static_cast<int>(lua_tointeger(lua, -1));

		obj->setDelayFrameCount(delayFrame);

		return 0;
	}

	//�����Ƃ���Model�N���X�̃C���X�^���X��gltf���f���̎�ނ������ԍ����󂯎��
	//Model�N���X��gltf���f����ݒ肷��
	static int glueSetGltfModel(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));
		std::string filePath = lua_tostring(lua, -1);

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			FileManager::GetInstance()->addLoadModelList(filePath, model);
			break;
		}


		return 0;
	}

	//�I�u�W�F�N�g�̍��W��ݒ肷��
	static int glueSetPos(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		{
			float x = static_cast<float>(lua_tonumber(lua, -3));
			float y = static_cast<float>(lua_tonumber(lua, -2));
			float z = static_cast<float>(lua_tonumber(lua, -1));
			obj->setPosition(glm::vec3(x, y, z));
			break;
		}
		}

		return 0;
	}

	//��]��ݒ肷�� �P�ʂ͓x���@
	static int glueSetRotate(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
			obj->rotate.x = static_cast<float>(lua_tonumber(lua, -3));
			obj->rotate.y = static_cast<float>(lua_tonumber(lua, -2));
			obj->rotate.z = static_cast<float>(lua_tonumber(lua, -1));
			break;
		}

		return 0;
	}

	//�X�P�[����ݒ肷��
	static int glueSetScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);

			glm::vec3 scale;
			for (int i = 0; i < 3; i++)
			{
				scale[i] = static_cast<float>(lua_tonumber(lua, -3 + i));
			}

			model->scale = scale;
			break;
		}

		return 0;
	}


	//gltf���f���̃x�[�X�J���[���㏑������
	static int glueSetBaseColor(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -5));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			float r = static_cast<float>(lua_tonumber(lua, -4));
			float g = static_cast<float>(lua_tonumber(lua, -3));
			float b = static_cast<float>(lua_tonumber(lua, -2));
			float a = static_cast<float>(lua_tonumber(lua, -1));
			glm::vec4 baseColor = glm::vec4(r, g, b, a);
			model->setBaseColor(baseColor);
		}

		return 0;
	}

	//�J�������Ǐ]����I�u�W�F�N�g��ݒ肷��
	static int glueBindCamera(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		obj->bindCamera(std::weak_ptr<Camera>(scene->camera));

		return 0;
	}

	//�I�u�W�F�N�g��AABB�R���C�_�[��ݒ肷��
	static int glueSetAABBColider(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->setColider();
			model->isMovable = static_cast<bool>(lua_toboolean(lua, -1));
			break;
		}

		return 0;
	}

	//�R���C�_�[�̃X�P�[����ݒ肷��
	static int glueSetColiderScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			std::shared_ptr<Colider> colider = model->getColider();
			if (colider)
			{
				float x = static_cast<float>(lua_tonumber(lua, -3));
				float y = static_cast<float>(lua_tonumber(lua, -2));
				float z = static_cast<float>(lua_tonumber(lua, -1));
				colider->scale = glm::vec3(x, y, z);
			}
			break;
		}

		return 0;
	}

	//�I�u�W�F�N�g�̃A�C�h�����ɍĐ�����A�j���[�V�����̖��O��ݒ肷��
	static int glueSetDefaultAnimationName(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->setDefaultAnimationName(std::string(lua_tostring(lua, -1)));
			break;
		}

		return 0;
	}

	//�I�u�W�F�N�g���]���d�͂̋�����ݒ肷��
	static int glueSetGravity(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -2));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->gravity = static_cast<float>(lua_tonumber(lua, -1));
			break;
		}

		return 0;
	}

	//���C�g�̌��̐F��ݒ肷��
	static int glueSetLightColor(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));
		glm::vec3 color =
		{
			static_cast<float>(lua_tonumber(lua, -3)),
			static_cast<float>(lua_tonumber(lua, -2)),
			static_cast<float>(lua_tonumber(lua, -1))
		};

		switch (obj->getObjNum())
		{
		case 3://�|�C���g���C�g�p
		{
			PointLight* pl = dynamic_cast<PointLight*>(obj);
			pl->color = color;
			break;
		}
		case 4://���s�����p
		{
			DirectionalLight* dl = dynamic_cast<DirectionalLight*>(obj);
			dl->color = color;
			break;
		}
		}

		return 0;
	}

	//���s�����̌��̕�����ݒ肷��
	static int glueSetLightDirection(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -4));
		glm::vec3 dir =
		{
			static_cast<float>(lua_tonumber(lua, -3)),
			static_cast<float>(lua_tonumber(lua, -2)),
			static_cast<float>(lua_tonumber(lua, -1))
		};

		switch (obj->getObjNum())
		{
		case 4://���s�����̂�
		{
			DirectionalLight* dl = dynamic_cast<DirectionalLight*>(obj);
			dl->direction = dir;
			break;
		}
		}

		return 0;
	}

	//�����̈�ڂ̃I�u�W�F�N�g��e�Ƃ��āA��̃I�u�W�F�N�g�ɐe�q�֌W��ݒ肷��
	static int glueBindObject(lua_State* lua)
	{
		Object* parent = static_cast<Object*>(lua_touserdata(lua, -2));
		Object* child = static_cast<Object*>(lua_touserdata(lua, -1));

		parent->bindObject(child);

		return 0;
	}

	//�I�u�W�F�N�g�̃��X�^�[�g���̍��W��ݒ肷��
	static int glueSetStartPoint(lua_State* lua)
	{
		float x, y, z;
		x = static_cast<float>(lua_tonumber(lua, -3));
		y = static_cast<float>(lua_tonumber(lua, -2));
		z = static_cast<float>(lua_tonumber(lua, -1));
		glm::vec3 startPoint = { x,y,z };

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		scene->setStartPoint(startPoint);

		return 0;
	}

	//�X�e�[�W�̉����̍��W��ݒ肷��
	static int glueSetLimitY(lua_State* lua)
	{
		float y;
		y = static_cast<float>(lua_tonumber(lua, -1));

		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		scene->setLimitY(y);

		return 0;
	}

	//�L���[�u�}�b�v��IBL�̂��Ƃ�HDRI�摜��ݒ肷��
	static int glueSetHDRIMap(lua_State* lua)
	{
		lua_getglobal(lua, "Scene");
		Scene* scene = static_cast<Scene*>(lua_touserdata(lua, -1));

		scene->setHDRIMap(lua_tostring(lua, -2));

		return 0;
	}

	//uv�𒲐�����悤�ɂ��āA�e�N�X�`���̈������΂���h��
	static int glueSetUVScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 1:
		case 2:
			Model * model = dynamic_cast<Model*>(obj);
			model->setUVScale();
			break;
		}

		return 0;
	}
};