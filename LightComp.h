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

struct PointLightComp
{
	PointLightUniform uniform;

	std::shared_ptr<GpuBuffer> uniformBuffer;

	std::shared_ptr<DescriptorSet> uniformDescriptorSet;

	std::shared_ptr<FrameBuffer> frameBuffer;

	std::shared_ptr<Texture> shadowMap;

	std::shared_ptr<DescriptorSet> texDescriptorSet;
};

struct DirectionLightUniform
{
	glm::vec3 position;
	glm::vec4 color;
};

struct DirectionLightComp
{
	DirectionLightUniform uniform;

	std::shared_ptr<GpuBuffer> uniformBuffer;

	std::shared_ptr<DescriptorSet> uniformDescriptorSet;

	std::shared_ptr<FrameBuffer> frameBuffer;

	std::shared_ptr<Texture> shadowMap;

	std::shared_ptr<DescriptorSet> texDescriptorSet;
};