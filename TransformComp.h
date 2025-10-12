#pragma once

#include"glm/glm.hpp"

struct TransformComp
{
	size_t entityID;

	glm::vec3 centerPos;//重心

	glm::vec3 position; //位置
	glm::vec3 rotation; //回転(オイラー角)
	glm::vec3 scale; //スケール

	glm::mat4 rotate;//回転行列
	glm::mat4 model;//モデル行列

	TransformComp()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);

		rotate = glm::mat4(1.0f);
		model = glm::mat4(1.0f);

		centerPos = position;
	}

	TransformComp(const size_t& entity)
	{
		entityID = entity;

		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);

		rotate = glm::mat4(1.0f);
		model = glm::mat4(1.0f);

		centerPos = position;
	}
};