#pragma once

#include"glm/glm.hpp"

#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>

struct TransformComp
{
	size_t entityID;

	glm::vec3 centerPos;//�d�S

	glm::vec3 position; //�ʒu
	glm::quat rotateQuat;//��]�N�H�[�^�j�I��
	glm::vec3 scale; //�X�P�[��

	glm::mat4 rotate;//��]�s��
	glm::mat4 model;//���f���s��

	TransformComp()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotateQuat = glm::quat(glm::vec4(0.0f));
		scale = glm::vec3(1.0f, 1.0f, 1.0f);

		rotate = glm::mat4(1.0f);
		model = glm::mat4(1.0f);

		centerPos = position;
	}

	TransformComp(const size_t& entity)
	{
		entityID = entity;

		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotateQuat = glm::quat(glm::vec4(0.0f));
		scale = glm::vec3(1.0f, 1.0f, 1.0f);

		rotate = glm::mat4(1.0f);
		model = glm::mat4(1.0f);

		centerPos = position;
	}
};