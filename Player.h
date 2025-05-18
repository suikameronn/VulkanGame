#pragma once

#include"UI.h"
#include "Model.h"
#include"Camera.h"
#include"FileManager.h"

//�v���C���[�L�����N�^�[�̃N���X
class Player : public Model
{
	//�J������y�������̉�]�p�x
	//�v���C���[�L�����N�^�[���J�����̐��ʕ����Ɍ������邽�߂̂���
	float cameraDirDeg;

	//�ړ����x
	float moveSpeed;
	//�W�����v�̍���
	float maxJumpHeight;
	//�L�[���͂��󂯎����
	bool controllable;

	//�L�[���͂���v���C���[���ړ�������
	glm::vec3 inputMove();

	//�Ə���UI�ɂ���
	std::shared_ptr<ImageData> targetImage;
	std::shared_ptr<UI> targetUI;

	//�_�����߂����̃J�����ʒu
	glm::vec3 aimingCameraOffsetSrc;
	glm::vec3 aimingCameraOffset;

	//�e�̃X�e�[�^�X
	//�e�̃X�s�[�h
	float bulletSpeed;
	//���C�L���X�g���̃��C�̒���
	float rayCastLength;
	//�e�̐i�s����
	glm::vec3 bulletDirection;
	//�e�̎˒�����
	float bulletDistanceLimit;

	bool aiming;
	//�e��Gltf���f���̃p�X
	std::string bulletFilePath;
	//�_�����߂�ۂ̏���
	void aim();
	//�e�𔭎�
	void shootBullet();

public:

	Player();
	Player(std::string luaScriptPath);

	//lua����Ăяo�����֐���o�^�����
	void registerGlueFunctions() override;

	//�ړ����x��ݒ肷��
	void setSpeed(float s);
	//�W�����v�̍�����ݒ肷��
	void setMaxJumpHeight(float height);
	//����t���[���̂ݎ��s
	void initFrameSetting() override;
	//�L�[���͂̎擾
	void customUpdate() override;
	//���X�^�[�g�n�_�փv���C���[�����[�v������
	void restart(glm::vec3 startPoint);

	//�^�[�Q�b�gUI�̉摜�ƃT�C�Y��ݒ�
	void setTargetUIImageAndScale(std::string filePath,float scale);
	//�_�����߂����̃J�����̈ʒu�̐ݒ�
	void setAimingCameraPos(glm::vec3 pos) { aimingCameraOffsetSrc = pos; }

	void Update() override;
	void updateTransformMatrix() override;//���W�ϊ��s��̍X�V
};

/*�ȉ��̐ÓI�֐���lua����Ăяo�����*/

//�ړ����x�̐ݒ�
static int glueSetSpeed(lua_State* lua)
{
	float speed = static_cast<float>(lua_tonumber(lua, -1));
	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));
	if (!player)
	{
		return 0;
	}

	player->setSpeed(speed);

	return 0;
}

//�W�����v�̍����̐ݒ�
static int glueSetJumpHeight(lua_State* lua)
{
	float height = static_cast<float>(lua_tonumber(lua, -1));
	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));

	if (!player)
	{
		return 0;
	}

	player->setMaxJumpHeight(height);

	return 0;
}

//�Ə�UI�ɂ��Đݒ�
static int glueSetTargetUI(lua_State* lua)
{
	std::string filePath = static_cast<std::string>(lua_tostring(lua, -2));
	float scale = static_cast<float>(lua_tonumber(lua, -1));

	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));

	if (!player)
	{
		return 0;
	}

	player->setTargetUIImageAndScale(filePath, scale);

	return 0;
}

//�_�����߂����̃J�����̈ʒu�̐ݒ�
static int glueSetAimingCameraPos(lua_State* lua)
{
	glm::vec3 pos;
	for (int i = 0; i < 3; i++)
	{
		pos[i] = static_cast<float>(lua_tonumber(lua, -3 + i));
	}

	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));

	if (!player)
	{
		return 0;
	}

	player->setAimingCameraPos(pos);

	return 0;
}