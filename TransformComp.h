#pragma once

#include"glm/glm.hpp"

struct TransfromComp
{
	glm::vec3 position; //�ʒu
	glm::vec3 rotation; //��](�I�C���[�p)
	glm::vec3 scale; //�X�P�[��

	TransfromComp()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
	}
};