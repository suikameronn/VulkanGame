#include"Scene.h"
#include"VulkanBase.h"

#include"Bullet.h"

Bullet::Bullet(float s, float length, glm::vec3 dir, glm::vec3 pos, float limit)
{
	objNum = ObjNum::BULLET;

	speed = 5.0f;

	position = pos;
	shootPos = pos;

	distanceLimit = limit;

	ray.direction = glm::normalize(dir);

	//�e��3D���f���̌������v�Z����
	calculateBulletAngle();

	updateTransformMatrix();
}

void Bullet::calculateBulletAngle()
{
	//y��
	float yaw = atan2(ray.direction.z, ray.direction.x);
	//x��
	float pitch = asin(ray.direction.y);
	//z���͉�]�����Ȃ�
	float roll = 0.0f;

	//3D���f���̉�]�p�x���v�Z
	rotate.x = pitch * (180.0f / glm::pi<float>());
	rotate.y = yaw * (180.0f / glm::pi<float>());
	rotate.z = roll * (180.0f / glm::pi<float>());
}

Bullet::~Bullet()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	vulkan->addDefferedDestructBuffer(ray.mappedBuffer);
}

//�����t���[���ɁA���C�L���X�g���̃��C�̒����Ȃǂ��v�Z
void Bullet::initFrameSetting()
{
	initFrame = false;

	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);

		createTransformTable();//lua�ɍ��W�ϊ��p�̍s��̕ϐ����쐬

		lua_getglobal(coroutine, "Update");

		int nresults;
		if (lua_resume(coroutine, nullptr, 0, &nresults) != LUA_YIELD)
		{
			std::cerr << "lua coroutine error" << std::endl;
		}
	}

	if (defaultAnimationName != "none")
	{
		//switchPlayAnimation();
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

	//�A�j���[�V�����s��̔z���������
	for (auto& matrices : jointMatrices)
	{
		std::fill(matrices.begin(), matrices.end(), glm::mat4(1.0f));
	}

	updateTransformMatrix();

	//R�؂ɂ��̃C���X�^���X��ǉ�
	scene->addModelToRTree(std::dynamic_pointer_cast<Model>(shared_from_this()));

	//���C�L���X�g���̃��C�̒����ƃ��C�̎n�_��ݒ�
	ray.length = abs(glm::dot(mbrMax - mbrMin, ray.direction));
	ray.origin = pivot - ray.direction * (ray.length / 2.0f);

	//gpu��Ƀo�b�t�@���쐬
	VulkanBase::GetInstance()->createUniformBuffer(&ray.mappedBuffer, ray.getSize());

	//gpu�Ƀ��C�̃f�[�^���R�s�[����
	ray.copyToGpuBuffer();

	//gpu��Ƀo�b�t�@���쐬
	VulkanBase::GetInstance()->setModelData(std::static_pointer_cast<Model>(shared_from_this()));
}

void Bullet::Update()
{
	if (abs(glm::length(shootPos - getPosition())) >= distanceLimit)
	{
		//���̏ꍇ�A�e���폜�\��ɂ���
		exist = false;
	}

	setLastFrameTransform();

	setPosition(getPosition() + ray.direction * speed);
}

void Bullet::updateTransformMatrix()//���W�ϊ��s����X�V
{
	transformMatrix = glm::translate(glm::mat4(1.0), position)
		* rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f), initScale * scale);

	//AABB���X�V
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//MBR���X�V
	calcMBR();

	pivot = (mbrMin + mbrMax) / 2.0f;

	//���C�̎n�_���X�V
	ray.origin = pivot - ray.direction * (ray.length / 2.0f);
	//gpu�Ƀ��C�̃f�[�^���R�s�[����
	ray.copyToGpuBuffer();

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;

	if (rNode)
	{
		//R�؂��X�V
		scene->updateObjectPos(std::dynamic_pointer_cast<Model>(shared_from_this()), rNode);
	}
}

void Bullet::collision(std::shared_ptr<Model> model)
{
	if (model->getObjNum() == ObjNum::PLAYER || model->getObjNum() == ObjNum::BULLET)
	{
		return;
	}

	//���C�L���X�g���ɁA���C���|���S���Ƀq�b�g�����ꍇ
	//�ȉ��̕ϐ��ɁA�Փ˂����|���S���ƃ��C�̋����Ɠ��������|���S����GltfNode���Ԃ��Ă���
	float distance;
	glm::vec3 faceNormal;
	GltfNode* node;

	VulkanBase::GetInstance()->startRaycast(ray, model, distance, faceNormal, &node);

	if (node)
	{
		//���̃C���X�^���X���폜�\��ɂ���
		model->notExist();
		this->notExist();
	}
}