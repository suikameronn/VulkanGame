#include"Scene.h"

#include "Player.h"

Player::Player()
{
	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();
}

Player::Player(std::string luaScriptPath)
{
	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();
}

//lua����Ăяo�����֐���o�^�����
void Player::registerGlueFunctions()
{
	lua_register(lua, "setSpeed", glueSetSpeed);//�ړ����x�̐ݒ�
	lua_register(lua, "setJumpHeight", glueSetJumpHeight);//�W�����v�̍����̐ݒ�
}

//�L�[���͂���v���C���[���ړ�������
glm::vec3 Player::inputMove()
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (!cameraObj.expired())
	{
		forward = cameraObj.lock()->forward;
		forward.y = 0.0f;
		right = cameraObj.lock()->right;
		cameraDirDeg = cameraObj.lock()->getTheta();
	}

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = -forward;
		rotate.y = cameraDirDeg;
	}
	else if (controller->getKey(GLFW_KEY_A))
	{
		moveDirec = -right;
		rotate.y = cameraDirDeg - 90.0f;
	}
	else if (controller->getKey(GLFW_KEY_D))
	{
		moveDirec = right;
		rotate.y = cameraDirDeg + 90.0f;
	}
	else if (controller->getKey(GLFW_KEY_S))
	{
		moveDirec = forward;
		rotate.y = cameraDirDeg + 180.0f;
	}
	else
	{
		moveDirec = { 0,0,0 };
	}

	if (moveDirec == glm::vec3(0.0f))
	{
		switchPlayAnimation("Idle");
	}
	else
	{
		moveDirec = glm::normalize(moveDirec) * moveSpeed;
		switchPlayAnimation("Running");
	}

	glm::vec3 groundNormal;
	if (isGround(groundNormal))
	{


		physicBase->cancelGravity();

		if (controller->getKey(GLFW_KEY_SPACE))
		{
			physicBase->addVelocity(up * maxJumpHeight);
		}
	}
	else
	{
		physicBase->addGravity();
		switchPlayAnimation("Floating");
	}

	return moveDirec;
}

//����t���[���̂ݎ��s
void Player::initFrameSetting()
{
	if (lua)
	{
		registerGlueFunctions();

		lua_pushlightuserdata(lua, this);
		lua_setglobal(lua, "Data");

		luaL_dofile(lua, luaPath.c_str());
	}

	if (defaultAnimationName != "none")
	{
		switchPlayAnimation();
	}

	if (colider)
	{
		colider->initFrameSettings();
	}

	//R�c���[�ɃI�u�W�F�N�g��ǉ�
	scene->addModelToRTree(this);
}

//�L�[���͂̎擾
void Player::customUpdate()
{
	glm::vec3 moveDirec = inputMove();
	setPosition(this->position + moveDirec);
}

//�ړ����x��ݒ肷��
void Player::setSpeed(float s)
{
	moveSpeed = s;
}

//�W�����v�̍�����ݒ肷��
void Player::setMaxJumpHeight(float height)
{
	maxJumpHeight = height;
}

//���X�^�[�g�n�_�փv���C���[�����[�v������
void Player::restart(glm::vec3 startPoint)
{
	setZeroVelocity();

	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);

	setPosition(startPoint);
}

void Player::Update()
{
	setLastFrameTransform();

	customUpdate();//�I�u�W�F�N�g�ŗL�̍X�V����

	setPosition(getPosition() + physicBase->getVelocity());

	playAnimation();//�A�j���[�V�����̍Đ�
}