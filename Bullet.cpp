#include"Scene.h"
#include"VulkanBase.h"

#include"Bullet.h"

Bullet::Bullet(float s, float length, glm::vec3 dir, glm::vec3 pos, float limit)
{
	{
		uvScale = false;

		scene = Scene::GetInstance();

		objNum = ObjNum::MODEL;
		tags.push_back(Tag::GROUND);

		uniformBufferChange = true;

		rotate.x = rotate.getRadian(0.0f);
		rotate.y = rotate.getRadian(0.0f);
		rotate.z = rotate.getRadian(0.0f);
		initScale = glm::vec3(1.0f);

		forward = glm::vec3{ 0,0,1 };
		right = glm::vec3{ 1,0,0 };
		up = glm::vec3{ 0,1,0 };

		childObjects.clear();
		spherePos = false;

		rotateSpeed = 0.1f;

		deltaTime = 0.0;
		animationChange = true;

		isMovable = false;
		colider = nullptr;

		gravity = 0.0f;

		defaultAnimationName = "Idle";

		mbrMin = glm::vec3(FLT_MAX);
		mbrMax = glm::vec3(-FLT_MAX);
	}

	speed = s;
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
	if (rNode)
	{
		//r木から自分を外しておく
		rNode->deleteObject(this);
	}
}

void Bullet::Update()
{
	if (abs(glm::length(shootPos - getPosition())) > distanceLimit)
	{

	}

	setLastFrameTransform();

	setPosition(getPosition() + direction * speed);
}