#pragma once

#include"PhysicMotion.h"

#include<glm/glm.hpp>
#include<glm/gtx/normalize_dot.hpp>

struct PhysicComp
{
	size_t entityID;

	//オブジェクトの位置フレーム前の位置
	glm::vec3 lastFramePosition;

	//1フレーム前の速度
	glm::vec3 lastFrameVeloctity;

	//現在の速度
	glm::vec3 velocity;

	//1フレーム前の角加速度
	glm::vec3 lastAngVelocity;

	//現在の角加速度
	glm::vec3 angVelocity;

	//慣性モーメント
	//float inertia;
	glm::mat3 inertia;

	//物理的特性
	PhysicParam param;

	//このフレーム、オブジェクトに加えられる力の配列
	std::list<std::shared_ptr<Force>> forceList;

	PhysicComp(const size_t& entity)
	{
		entityID = entity;

		lastFrameVeloctity = glm::vec3(0.0f);
		velocity = glm::vec3(0.0f);

		lastAngVelocity = glm::vec3(0.0f);
		angVelocity = glm::vec3(0.0f);

		param = PhysicParam();

		forceList.clear();
	};
};