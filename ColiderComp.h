#pragma once

#include"PhysicMotion.h"

//コライダーの種類
enum class ColliderType
{
	Box
};

//衝突時の法線などを記録
struct CollisionInfo
{
	//自身の速度
	glm::vec3 myVelocity;

	//相手の速度
	glm::vec3 partnerVelocity;

	//法線
	glm::vec3 normal;

	//衝突した相手の物理的特性
	PhysicParam param;

	CollisionInfo()
	{
		normal = glm::vec3(0.0f);
	}
};

struct ColiderComp
{
	size_t entityID;

	//コライダーのID
	size_t ID;

	//コライダーのタイプ
	ColliderType type;

	//コライダーのサイズや半径などのパラメータ
	//現在はボックスのみ
	//この値は、3Dモデルのスケールにさらに、掛け合わされる
	//実際の3Dモデルよりもコライダーのみ大きくしたいときよう
	float size;

	bool freeze;

	//コライダーの各オフセット
	glm::vec3 offsetPos;
	glm::vec3 offsetScale;

	CollisionInfo collision;

	ColiderComp(const size_t& entity)
	{
		entityID = entity;

		ID = 0;
		type = ColliderType::Box;
		size = 1.0f;

		offsetPos = glm::vec3(0.0f);
		offsetScale = glm::vec3(0.0f);

		collision = CollisionInfo{};
	};
};