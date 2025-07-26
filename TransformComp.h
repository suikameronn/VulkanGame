#pragma once

#include"glm/glm.hpp"

struct TransformComp
{
	glm::vec3 position; //位置
	glm::vec3 rotation; //回転(オイラー角)
	glm::vec3 scale; //スケール

	TransformComp()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
	}
};