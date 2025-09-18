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
	size_t entityID;

	float aspect;
	float viewAngle;
	float zNear, zFar;

	float theta;
	float phi;

	//カメラから見た正面と右のベクトル
	glm::vec3 forward;
	glm::vec3 right;

	CameraUniform matrices;

	std::shared_ptr<GpuBuffer> uniform;
	std::shared_ptr<DescriptorSet> descriptorSet;

	CameraComp(const size_t& entity)
	{
		entityID = entity;

		aspect = 900.0f / 600.0f;
		viewAngle = 45.0f;
		zNear = 0.1f;
		zFar = 1000.0f;

		theta = 125.0f;
		phi = -89.0f;

		forward = { 0.0f,0.0f,1.0f };
		right = { 1.0f,0.0f,0.0f };

		matrices.position = glm::vec3(0.0f);
		matrices.view = glm::mat4(1.0f);
		matrices.proj = glm::mat4(1.0f);
	}
};