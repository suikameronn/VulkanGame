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

	//�e�̌�����i�s�����ɍ��킹��
	calculateBulletAngle();

	updateTransformMatrix();

	ray.direction = glm::normalize(dir);
}

void Bullet::calculateBulletAngle()
{
	//y��
	float yaw = atan2(ray.direction.z, ray.direction.x);
	//x��
	float pitch = asin(ray.direction.y);
	//z���͉�]�����Ȃ�
	float roll = 0.0f;

	//�x�ɕϊ����Ă���
	rotate.x = pitch * (180.0f / glm::pi<float>());
	rotate.y = yaw * (180.0f / glm::pi<float>());
	rotate.z = roll * (180.0f / glm::pi<float>());
}

Bullet::~Bullet()
{
}

//����t���[���̐ݒ�
void Bullet::initFrameSetting()
{
	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);

		createTransformTable();//�I�u�W�F�N�g�̃g�����X�t�H�[����lua�ɑ���

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

	//�A�j���[�V�����s��̏�����
	for (auto& matrices : jointMatrices)
	{
		std::fill(matrices.begin(), matrices.end(), glm::mat4(1.0f));
	}

	updateTransformMatrix();

	//�V�[���S�̂�R-tree�ɂ��̃I�u�W�F�N�g��ǉ�

	/////shared_from_this�łȂ����G���[���ł�
	scene->addModelToRTree(std::dynamic_pointer_cast<Model>(shared_from_this()));

	//���C�L���X�g���̃��C�̐ݒ�
	ray.length = abs(glm::dot(max - min, ray.direction));
	ray.origin = pivot - ray.direction * (ray.length / 2.0f);
}

void Bullet::Update()
{
	if (abs(glm::length(shootPos - getPosition())) >= distanceLimit)
	{
		//�e�ۂ̃I�u�W�F�N�g���폜����
		exist = false;
	}

	setLastFrameTransform();

	setPosition(getPosition() + ray.direction * speed);
}

void Bullet::collision(std::shared_ptr<Model> model)
{
	if (model->getObjNum() == ObjNum::PLAYER || model->getObjNum() == ObjNum::BULLET)
	{
		return;
	}

	std::cout << "Bulllet" << std::endl;
}