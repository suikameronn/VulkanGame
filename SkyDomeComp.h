#pragma once

#include<glm/glm.hpp>

#include"SkyDomeFactory.h"

struct SkyDomeComp
{
	size_t entityID;

	//�t�@�N�g���[����擾����
	size_t ID;

	SkyDomeComp(const size_t& entity)
	{
		entityID = entity;
	}
};