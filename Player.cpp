#include"Scene.h"
#include"VulkanBase.h"
#include"Bullet.h"

#include "Player.h"

#include"GameManager.h"

Player::Player()
{
	objNum = ObjNum::PLAYER;

	aiming = false;

	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();

	bulletFilePath = "beamBullet.glb";
	bulletSpeed = 1.0f;
	rayCastLength = 1.0f;
	bulletDistanceLimit = 1000.0f;
}

Player::Player(std::string luaScriptPath)
{
	aiming = false;

	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();

	bulletFilePath = "beamBullet.glb";
	bulletSpeed = 1.0f;
	rayCastLength = 1.0f;
	bulletDistanceLimit = 1000.0f;
}

//lua����Ăяo�����֐���o�^�����
void Player::registerGlueFunctions()
{
	lua_register(lua, "setSpeed", glueSetSpeed);//�ړ����x�̐ݒ�
	lua_register(lua, "setJumpHeight", glueSetJumpHeight);//�W�����v�̍����̐ݒ�
	lua_register(lua, "setTargetUI", glueSetTargetUI);//�Ə�UI�̉摜�ƃT�C�Y��ݒ�
	lua_register(lua, "setAimingCameraPos", glueSetAimingCameraPos);//�_�����߂����̃J�����̈ʒu�̐ݒ�
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

	if (!aiming)
	{
		if (controller->getKey(GLFW_KEY_W))
		{
			moveDirec = forward;
			setRotate(rotate.x, cameraDirDeg, rotate.z);
		}
		else if (controller->getKey(GLFW_KEY_A))
		{
			moveDirec = -right;
			setRotate(rotate.x, cameraDirDeg - 90.0f, rotate.z);
		}
		else if (controller->getKey(GLFW_KEY_D))
		{
			moveDirec = right;
			setRotate(rotate.x, cameraDirDeg + 90.0f, rotate.z);
		}
		else if (controller->getKey(GLFW_KEY_S))
		{
			moveDirec = -forward;
			setRotate(rotate.x, cameraDirDeg + 180.0f, rotate.z);
		}
		else
		{
			moveDirec = { 0,0,0 };
		}

		if (moveDirec == glm::vec3(0.0f))
		{
			switchPlayAnimation("Idle_gunMiddle");
		}
		else
		{
			moveDirec = glm::normalize(moveDirec) * moveSpeed;
			switchPlayAnimation("run");
		}
	}
	else
	{
		if (controller->getKey(GLFW_KEY_W))
		{
			moveDirec = forward;
			switchPlayAnimation("walk_back_shoot");
		}
		else if (controller->getKey(GLFW_KEY_A))
		{
			moveDirec = -right;
			switchPlayAnimation("walk_left_shoot");
		}
		else if (controller->getKey(GLFW_KEY_D))
		{
			moveDirec = right;
			switchPlayAnimation("walk_right_shoot");
		}
		else if (controller->getKey(GLFW_KEY_S))
		{
			moveDirec = -forward;
			switchPlayAnimation("walk_front_shoot");
		}
		else
		{
			moveDirec = { 0,0,0 };
		}

		if (moveDirec == glm::vec3(0.0f))
		{
			switchPlayAnimation("aiming");
		}
		else
		{
			moveDirec = glm::normalize(moveDirec) * moveSpeed;
		}
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
		currentPlayAnimationName = defaultAnimationName;
		nextPlayAnimationName = defaultAnimationName;
	}

	if (hasColiderFlag)
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel, trigger));
		colider->initFrameSettings(glm::vec3(1.0f));
	}

	initMin = min;
	initMax = max;

	//�A�j���[�V�����s��̏�����
	for (auto& matrices : jointMatrices)
	{
		std::fill(matrices.begin(), matrices.end(), glm::mat4(1.0f));
	}

	updateTransformMatrix();

	//R�c���[�ɃI�u�W�F�N�g��ǉ�
	scene->addModelToRTree(std::dynamic_pointer_cast<Model>(shared_from_this()));

	aimingCameraOffset = aimingCameraOffsetSrc;

	//�J�����̈ʒu�𒲐�
	sendPosToCamera(((min + max) / 2.0f));

	//gpu��Ƀo�b�t�@���쐬
	VulkanBase::GetInstance()->setModelData(std::static_pointer_cast<Model>(shared_from_this()));

	//gpu��Ƀ��C�L���X�g�g�p���̃o�b�t�@���쐬
	VulkanBase::GetInstance()->createUniformBuffer(&ray.mappedBuffer, ray.getSize());
}

//�L�[���͂̎擾
void Player::customUpdate()
{
	Controller* controller = Controller::GetInstance();

	glm::vec3 moveDirec = inputMove();

	setPosition(this->position + moveDirec);

	if (controller->getMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
	{
		aiming = true;
		aim();
	}
	else if(aiming)
	{
		aiming = false;

		//�J�����̈ʒu�����Z�b�g
		cameraObj.lock()->posReset();
		//�J�����̋��ʈړ����J�n
		cameraObj.lock()->sphereMove = true;
		//�J�����̒Ǐ]��������W�����̃I�u�W�F�N�g�̒��S�ɂ���
		cameraObj.lock()->setParentPos(((min + max) / 2.0f) - cameraObj.lock()->getViewTarget());

		targetUI->setVisible(false);

		//�����𔼓����Ń����_�����O����̂���߂�
		fragParam.alphaness = -1.0f;
	}
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

	setRotate(0.0f, 0.0f, 0.0f);

	setPosition(startPoint);
}

//�^�[�Q�b�gUI�̉摜�ƃT�C�Y��ݒ�
void Player::setTargetUIImageAndScale(std::string filePath, float scale)
{
	GameManager* manager = GameManager::GetInstance();

	targetImage = FileManager::GetInstance()->loadImage(filePath);

	targetUI = std::shared_ptr<UI>(new UI(targetImage));
	targetUI->setScale(scale);
	targetUI->setPosition(glm::vec2(manager->getWindowWidth() / 2.0f, manager->getWindowHeight() / 2.0f));
	targetUI->setVisible(false);//�f�t�H���g�Ŕ�\���Ɏw�肨��
	targetUI->setTransparent(true);

	scene->sceneUI.push_back(targetUI);
}

void Player::Update()
{
	setLastFrameTransform();

	customUpdate();//�I�u�W�F�N�g�ŗL�̍X�V����

	setPosition(getPosition() + physicBase->getVelocity());

	playAnimation();//�A�j���[�V�����̍Đ�
}

//���W�ϊ��s��̍X�V
void Player::updateTransformMatrix()
{
	glm::mat4 tr = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix();
	transformMatrix = tr * glm::scale(glm::mat4(1.0f), initScale * scale);

	//AABB�̍X�V
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//�_�����߂����̃J�����̈ʒu�𒲐�
	aimingCameraOffset = tr * glm::vec4(aimingCameraOffsetSrc, 1.0f);

	//MBR�̍X�V
	calcMBR();

	pivot = (mbrMin + mbrMax) / 2.0f;

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;

	if (rNode)
	{
		//R�c���[��̃I�u�W�F�N�g�̈ʒu���X�V����
		scene->updateObjectPos(std::dynamic_pointer_cast<Model>(shared_from_this()), rNode);
	}
}

void Player::aim()
{
	Controller* controller = Controller::GetInstance();

	//�Ə�UI�̕\��
	targetUI->setVisible(true);

	//�J����������̈ʒu�ɐݒ肷��
	cameraObj.lock()->setPosition(aimingCameraOffset);

	cameraObj.lock()->setParentPos((aimingCameraOffset + forward * 1.1f) - cameraObj.lock()->getViewTarget());

	cameraObj.lock()->sphereMove = false;

	{
		//�v���C���[�̌�����_�����߂������Ɍ�����

		//y��
		float yaw = atan2(-forward.z, -forward.x);
		//x��
		float pitch = asin(-forward.y);
		//z���͉�]�����Ȃ�
		float roll = 0.0f;

		float radToDeg = (180.0f / glm::pi<float>());

		//�x�ɕϊ����Ă���
		setRotate(pitch * radToDeg,yaw * radToDeg,roll * radToDeg);
	}

	//�������g�𔼓����Ń����_�����O����悤�ɂ���
	fragParam.alphaness = 0.7f;

	if (controller->getMouseButton(GLFW_MOUSE_BUTTON_LEFT))
	{
		shootBullet();
	}
}

void Player::shootBullet()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	//�e�̐i�s����
	bulletDirection = forward;

	std::shared_ptr bullet =
		std::shared_ptr<Bullet>(new Bullet(bulletSpeed, rayCastLength
			, bulletDirection, pivot, bulletDistanceLimit));

	//�X�P�[���̐ݒ�
	bullet->scale = glm::vec3(20.0f, 1.0f, 1.0f);

	//3D���f����ݒ�
	bullet->setgltfModel(Storage::GetInstance()->getgltfModel(bulletFilePath));

	//�R���C�_�[��ݒ�(�g���K�[�̐ݒ�ɂ���)
	bullet->setColider(true);

	//�����_�����O�̃��X�g�ɒǉ�
	scene->sceneModels.push_back(bullet);
}

void Player::collision(std::shared_ptr<Model> model)
{
	if (model->containTag(Tag::GROUND) && isMovable)
	{
		//���ɂ���I�u�W�F�N�g��

		float distance = 0.0f;
		GltfNode* node = nullptr;
		glm::vec3 normal;

		ray.origin = getPivot();
		ray.length = (getPivot().y - position.y) * 1.2f;
		ray.direction = -up;
		ray.copyToGpuBuffer();

		VulkanBase::GetInstance()->startRaycast(ray, model, distance, normal, &node);

		normal = glm::normalize(normal);

		//��������ׂ�y���W
		float destY;
		destY = position.y - distance - (position.y - ray.origin.y);

		std::cout << destY << std::endl;

		if (!node)
		{
			//�n�ʂ̃|���S���̏�ɂ͗����Ă��Ȃ�����
			//���̐�̏Փ˂�O��Ƃ��������͍s��Ȃ�

			return;
		}

		//�n�ʂ̃|���S���̏�ɗ����Ă����ꍇ

		//y���W�݂̂�ς��āA�|���S���ɑ�������
		setPosition(glm::vec3(getPosition().x, destY, getPosition().z));

		//�ʖ@�������]���擾����

	}
}