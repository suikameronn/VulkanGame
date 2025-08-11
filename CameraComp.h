#pragma once

#include"glm/glm.hpp"

#include"DescriptorSetFactory.h"

struct CameraUniform
{
	glm::vec3 position;
	alignas(16) glm::mat4 view;
	glm::mat4 proj;

	CameraUniform()
	{
		position = glm::vec3(0.0f);
		view = glm::mat4(1.0f);
		proj = glm::mat4(1.0f);
	}
};

struct CameraComp
{
	float aspect;
	float viewAngle;
	float zNear, zFar;

	CameraUniform matrices;

	std::shared_ptr<GpuBuffer> uniform;
	std::shared_ptr<DescriptorSet> descriptorSet;

	CameraComp()
	{
		aspect = 900.0f / 600.0f;
		viewAngle = 45.0f;
		zNear = 0.1f;
		zFar = 1000.0f;

		matrices.position = glm::vec3(0.0f);
		matrices.view = glm::mat4(1.0f);
		matrices.proj = glm::mat4(1.0f);
	}
};