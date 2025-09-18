#pragma once

#include"GltfModelComp.h"

struct ModelMat
{
	glm::vec3 scale;
	alignas(16) glm::mat4 matrix;

	ModelMat()
	{
		scale = glm::vec3(1.0f);
		matrix = glm::mat4(1.0f);
	}
};

struct NodeAnimMat
{
	alignas(16) glm::mat4 nodeMatrix;
	alignas(16) glm::mat4 matrix;
	alignas(16) int boneCount;

	NodeAnimMat()
	{
		nodeMatrix = glm::mat4(1.0f);
		matrix = glm::mat4(1.0f);
		boneCount = 0;
	}
};

struct BoneMat
{
	alignas(16) std::array<glm::mat4, 128> boneMatrix;

	BoneMat()
	{
		std::fill(boneMatrix.begin(), boneMatrix.end(), glm::mat4(1.0f));
	}
};

struct MeshRendererComp
{
	size_t entityID;

	ModelMat model;

	std::vector<NodeAnimMat> nodeAnim;

	BoneMat bone;

	std::shared_ptr<GpuBuffer> modelMatBuffer;

	std::vector<std::shared_ptr<GpuBuffer>> nodeAnimBuffer;

	std::shared_ptr<GpuBuffer> boneMatBuffer;

	std::vector<std::shared_ptr<DescriptorSet>> modelAnimDesc;

	MeshRendererComp(const size_t& entity)
	{
		entityID = entity;
	}
};