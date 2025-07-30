#pragma once

#include<iostream>
#include<glm/glm.hpp>

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"
#include"TextureFactory.h"

struct PointLightUniform
{
	glm::vec3 position;
	glm::vec4 color;
};

struct PointLight
{
	PointLightUniform uniform;

	std::shared_ptr<GpuBuffer> uniformBuffer;

	std::shared_ptr<Texture> shadowMap;

	std::shared_ptr<DescriptorSet> descriptorSet;
};

struct DirectionLightUniform
{
	glm::vec3 position;
	glm::vec4 color;
};

struct DirectionLight
{
	DirectionLightUniform uniform;

	std::shared_ptr<GpuBuffer> uniformBuffer;

	std::shared_ptr<Texture> shadowMap;

	std::shared_ptr<DescriptorSet> descriptorSet;
};