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

	//弾の向きを進行方向に合わせる
	calculateBulletAngle();

	updateTransformMatrix();
}

void Bullet::calculateBulletAngle()
{
	//y軸
	float yaw = atan2(direction.z, direction.x);
	//x軸
	float pitch = asin(direction.y);
	//z軸は回転させない
	float roll = 0.0f;

	//度に変換しておく
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
		//弾丸のオブジェクトを削除する
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