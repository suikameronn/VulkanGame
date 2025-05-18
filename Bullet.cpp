#include"Scene.h"
#include"VulkanBase.h"

#include"Bullet.h"

Bullet::Bullet(float s, float length, glm::vec3 dir, glm::vec3 pos, float limit)
{
	objNum = ObjNum::BULLET;

	speed = 100.0f;
	rayLength = length;
	direction = glm::normalize(dir);

	position = pos;
	shootPos = pos;

	distanceLimit = limit;

	scale = glm::vec3(100.0f);

	//�e�̌�����i�s�����ɍ��킹��
	calculateBulletAngle();

	updateTransformMatrix();
}

void Bullet::calculateBulletAngle()
{
	//y��
	float yaw = atan2(direction.z, direction.x);
	//x��
	float pitch = asin(direction.y);
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

void Bullet::Update()
{
	if (abs(glm::length(shootPos - getPosition())) > distanceLimit)
	{
		//�e�ۂ̃I�u�W�F�N�g���폜����
		exist = false;
	}

	setLastFrameTransform();

	setPosition(getPosition() + direction * speed);
}

void Bullet::collision(std::weak_ptr<Model> collideObj)
{
	std::cout << "Collision Model" << std::endl;
}

void Bullet::collision(std::weak_ptr<Player> player)
{
	std::cout << "Collision player" << std::endl;
}