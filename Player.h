#include "Model.h"
#include"Camera.h"

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