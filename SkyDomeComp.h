#pragma once

#include<glm/glm.hpp>

#include"SkyDomeFactory.h"

struct SkyDomeComp
{
	size_t entityID;

	//ファクトリーから取得する
	size_t ID;

	SkyDomeComp(const size_t& entity)
	{
		entityID = entity;
	}
};