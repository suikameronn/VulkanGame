#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define PI 3.14159265359

#include<iostream>
#include<vector>
#include<math.h>
#include<memory>

#include<vulkan/vulkan.h>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <lua.hpp>
#include <lualib.h>
#include <lauxlib.h>

#include"EnumList.h"
#include"Controller.h"
#include"Camera.h"
#include"PhysicBase.h"

//���_�o�b�t�@�[�ƃC���f�b�N�X�o�b�t�@�̍\����
//Model�N���X������
struct BufferObject
{
	VkBuffer vertBuffer;
	VkDeviceMemory vertHandler;

	VkBuffer indeBuffer;
	VkDeviceMemory indeHandler;
};

//�p�C�v���C�����C�A�E�g�ƃp�C�v���C��
struct DescriptorInfo
{
	VkPipelineLayout pLayout;
	VkPipeline pipeline;
};

//uniform buffer���L�^���邽�߂̃o�b�t�@�[
struct MappedBuffer
{
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;

	void destroy(VkDevice& device)
	{
		vkDestroyBuffer(device, uniformBuffer, nullptr);
		vkFreeMemory(device, uniformBufferMemory, nullptr);
		uniformBufferMapped = nullptr;
	}
};

//�����_�����O���̃v���~�e�B�u��descriptorSet
struct DescSetData
{
	VkPrimitiveTopology topology;
	VkDescriptorSet descriptorSet;
};

//��]�̍\����
//�e���ɓx���@�P�ʂ̉�]�p�x���w�肷��
struct Rotate
{
	float x;
	float y;
	float z;

	//���W�A���֕ϊ�
	float getRadian(float deg)
	{
		return static_cast<float>(deg * (PI / 180.0f));
	}

	//��]�s��̎擾
	glm::mat4 getRotateMatrix()
	{
		return glm::rotate(glm::mat4(1.0f), getRadian(z), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(y), glm::vec3(0.0f, -1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), getRadian(x), glm::vec3(1.0f, 0.0f, 0.0f));
	}
};

//���ׂẴI�u�W�F�N�g�̌p�����̃N���X�A���W�̐ݒ�Alua�X�N���v�g�̐ݒ�Ȃǂ�S��
class Object
{
protected:

	//�s���p�^�[���plua�X�N���v�g�̉��z�}�V���ɃA�N�Z�X��ϐ�
	lua_State* lua;
	//�X�N���v�g�̃R���[�`���ĊJ�p
	lua_State* coroutine;
	//lua�X�N���v�g�̃p�X
	std::string luaPath;

	//�I�u�W�F�N�g�̃^�C�v������
	ObjNum objNum;
	//�^�O������
	std::vector<Tag> tags;

	//���������̌v�Z
	std::unique_ptr<PhysicBase> physicBase;

	//�J������Ǐ]������ꍇ�A���̕ϐ��ɃJ�����ւ̎Q�Ƃ���������
	std::weak_ptr<Camera> cameraObj;
	//�����̃I�u�W�F�N�g�ɒǏ]����q�I�u�W�F�N�g�̔z��
	std::vector<Object*> childObjects;

	//��]���x
	float rotateSpeed;

	float length;

	//�q�I�u�W�F�N�g�Ɏ��g�̈ړ��Ȃǂ�Ǐ]�����邽�߂�
	//�O��̃t���[���̎��g�̍��W�Ȃǂ��L�^���Ă���
	//�O��t���[���̎����̍��W
	glm::vec3 lastPos;
	//��]
	Rotate lastRotate;
	//�X�P�[��
	glm::vec3 lastScale;

	//���݂̍��W
	glm::vec3 position;

	//�L�[���͂̎󂯎��
	virtual glm::vec3 inputMove();

public:
	Object();

	//���g�̃I�u�W�F�N�g�̃^�C�v��Ԃ�
	ObjNum getObjNum() { return objNum; }
	//�w�肳�ꂽ�^�O�������Ă��邩�ǂ�����Ԃ�
	bool containTag(Tag tag);

	//�s���p�^�[���p��lua�X�N���v�g��ݒ肷�� ���s�͂��Ȃ�
	void setLuaScript(std::string path);

	//�e�I�u�W�F�N�g�𒆐S�ɂ������ʍ��W��𓮂����ǂ���
	bool spherePos;

	//�ړ��Ȃǂɂ����W�ϊ��s����X�V����K�v�����邩�ǂ���
	bool uniformBufferChange;

	//��]�̏��
	Rotate rotate;

	//�e�I�u�W�F�N�g�Ƃ̋���
	float posOffSet;

	//�I�u�W�F�N�g�̐��ʁA�E�A��̃x�N�g��
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	//���g�̃��f���s��
	glm::mat4 transformMatrix;

	//�����̃I�u�W�F�N�g���q�I�u�W�F�N�g�Ƃ��Đݒ肷��
	void bindObject(Object* obj);
	//�J�����������ɒǏ]������
	void bindCamera(std::weak_ptr<Camera> camera);
	//�����̈ړ����q�I�u�W�F�N�g�ɑ���A�q�I�u�W�F�N�g�������̈ړ��ɒǏ]������
	void sendPosToChildren();
	//�e�I�u�W�F�N�g�̈ړ����󂯎��A�e�I�u�W�F�N�g�̈ړ��ɒǏ]����
	void setParentPos(glm::vec3 lastPos, glm::vec3 currentPos);

	//���W�̐ݒ�
	virtual void setPosition(glm::vec3 pos);
	//���W�̎擾
	glm::vec3 getPosition();

	//�O��t���[���̃g�����X�t�H�[�����擾����
	glm::vec3 getLastPosition();
	Rotate getLastRotate();
	glm::vec3 getLastScale();
	virtual void setLastFrameTransform();//��O�̃t���[���̍��W�Ȃǂ̃f�[�^��ݒ�

	//���f���s����擾
	glm::mat4 getTransformMatrix();

	//lua�̉��z�}�V���ɁA���g�̍��W�Ɖ�]���L�^����ϐ����쐬
	virtual void createTransformTable();
	//����t���[���̂ݎ��s�Alua�X�N���v�g�����s����
	virtual void initFrameSetting();
	//lua����Ăяo�����ÓI�֐��̓o�^
	virtual void registerGlueFunctions();
	//���f���s��̍X�V
	virtual void updateTransformMatrix() {};
	//�X�V����
	virtual void Update();
	//���قȍX�V����(�ʏ�͂Ȃ�)
	virtual void customUpdate() {};
	//lua�X�N���v�g�Ɏ��g�̌��݂̍��W�A��]�̒l�𑗂�
	virtual void sendTransformToLua();
	//lua�X�N���v�g����A���W�A��]�̒l���󂯎��
	virtual void receiveTransformFromLua();
};

/*�ȉ��̊֐���lua�X�N���v�g������s�����*/

namespace glueObjectFunction//Object�N���X�p��glue�֐�
{

	//���W�̐ݒ�
	static int glueSetPos(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		{
			float x = static_cast<float>(lua_tonumber(lua, -4));
			float y = static_cast<float>(lua_tonumber(lua, -3));
			float z = static_cast<float>(lua_tonumber(lua, -2));
			obj->setPosition(glm::vec3(x, y, z));
			break;
		}
		}

		return 0;
	}

	//��]�̐ݒ�
	static int glueSetRotate(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 0:
		case 1:
			obj->rotate.x = static_cast<float>(lua_tonumber(lua, -4));
			obj->rotate.y = static_cast<float>(lua_tonumber(lua, -3));
			obj->rotate.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}