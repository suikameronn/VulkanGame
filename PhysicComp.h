#pragma once

#include"PhysicMotion.h"

#include<glm/glm.hpp>
#include<glm/gtx/normalize_dot.hpp>

struct PhysicComp
{
	size_t entityID;

	//�I�u�W�F�N�g�̈ʒu�t���[���O�̈ʒu
	glm::vec3 lastFramePosition;

	//1�t���[���O�̑��x
	glm::vec3 lastFrameVeloctity;

	//���݂̑��x
	glm::vec3 velocity;

	//1�t���[���O�̊p�����x
	glm::vec3 lastAngVelocity;

	//���݂̊p�����x
	glm::vec3 angVelocity;

	//�������[�����g
	//float inertia;
	glm::mat3 inertia;

	//�����I����
	PhysicParam param;

	//���̃t���[���A�I�u�W�F�N�g�ɉ�������͂̔z��
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