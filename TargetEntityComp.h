#pragma once

#include<glm/glm.hpp>

struct TargetEntityComp
{
	size_t targetEntity;

	glm::vec3 offset;

	TargetEntityComp()
	{
		targetEntity = UINT_MAX;

		offset = glm::vec3(0.0f);
	}
};