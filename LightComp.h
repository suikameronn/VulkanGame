#pragma once

#include<iostream>
#include<glm/glm.hpp>

#include"TextureFactory.h"

struct PointLight
{
	glm::vec3 position;
	glm::vec4 color;

	std::shared_ptr<Texture> shadowMap;
};

struct DirectionLight
{
	glm::vec3 dir;
	glm::vec4 color;

	std::shared_ptr<Texture> shadowMap;
};