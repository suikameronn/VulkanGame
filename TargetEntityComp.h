#pragma once

#include<glm/glm.hpp>

struct TargetEntityComp
{
	size_t entityID;

	size_t targetEntity;

	glm::vec3 offset;

	TargetEntityComp(const size_t& entity)
	{
		entityID = entity;

		targetEntity = UINT_MAX;

		offset = glm::vec3(0.0f);
	}
};