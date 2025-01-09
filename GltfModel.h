#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include<iostream>
#include<vector>
#include<array>
#include<map>
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include"Material.h"

struct Vertex;
struct Primitive;
struct Mesh;
struct Skin;
struct GltfNode;
struct Animation;
struct BoundingBox;

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	uint32_t index;
	glm::ivec4 boneID1;

	glm::vec4 weight1;

	Vertex()
	{
		pos = glm::vec3(0.0);
		color = glm::vec3(0.0);
		texCoord = glm::vec2(0.0);
		normal = glm::vec3(0.0);

		index = 0;

		for (uint32_t i = 0; i < 4; i++)
		{
			boneID1[i] = 0;
			weight1[i] = 0.0f;
		}
	};

	void addBoneData(uint32_t boneID, float weight)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (weight1[i] == 0.0f)
			{
				boneID1[i] = boneID;
				weight1[i] = weight;

				return;
			}
		}
	}
};

struct BoundingBox
{
	glm::vec3 min;
	glm::vec3 max;
	bool valid = false;

	BoundingBox() {};
	BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {
	};
	BoundingBox getAABB(glm::mat4 m) {
		glm::vec3 min = glm::vec3(m[3]);
		glm::vec3 max = min;
		glm::vec3 v0, v1;

		glm::vec3 right = glm::vec3(m[0]);
		v0 = right * this->min.x;
		v1 = right * this->max.x;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 up = glm::vec3(m[1]);
		v0 = up * this->min.y;
		v1 = up * this->max.y;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 back = glm::vec3(m[2]);
		v0 = back * this->min.z;
		v1 = back * this->max.z;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		return BoundingBox(min, max);
	}
};

struct Primitive
{
	int primitiveIndex;
	int firstIndex;
	int indexCount;
	int vertexCount;
	std::shared_ptr<Material> material;
	bool hasIndices;
	BoundingBox bb;

	Primitive(int primitiveIndex,int firstIndex, int indexCount, int vertexCount,std::shared_ptr<Material> material)
		: primitiveIndex(primitiveIndex),firstIndex(firstIndex), indexCount(indexCount), vertexCount(vertexCount), material(material) {
		hasIndices = indexCount > 0;
	}
	void setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}
};

struct Skin
{
	std::string name;
	GltfNode* skeletonRoot = nullptr;
	std::vector<glm::mat4> inverseBindMatrices;
	std::vector<GltfNode*> joints;
};

struct Mesh
{
	//Meshの持つ頂点をバッファから参照するためのindex
	uint32_t meshIndex;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Primitive> primitives;
	DescriptorInfo descriptorInfo;

	BoundingBox aabb;
	BoundingBox bb;

	void setBoundingBox(glm::vec3 min, glm::vec3 max) {
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}
};

struct GltfNode
{
	GltfNode* parent;
	uint32_t index;
	std::vector<GltfNode*> children;
	glm::mat4 matrix;
	std::string name;
	Mesh* mesh;
	Skin* skin;
	int skinIndex = -1;
	int globalHasSkinNodeIndex = 0;
	glm::vec3 translation{};
	glm::vec3 scale{ 1.0f };
	glm::quat rotation{};

	BoundingBox bvh;
	BoundingBox aabb;
	
	GltfNode()
	{
		parent = nullptr;
		index = 0;
		mesh = nullptr;
		skin = nullptr;
	}

	~GltfNode()
	{
		if (mesh)
		{
			delete mesh;
		}
	}

	glm::mat4 localMatrix()
	{
		return glm::translate(glm::mat4(1.0f), translation) * 
			glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
	}

	glm::mat4 getMatrix()
	{
		glm::mat4 mat = localMatrix();
		GltfNode* p = parent;
		while (p)
		{
			mat = p->localMatrix() * mat;
			p = p->parent;
		}
		return mat;
	}

	void update(std::array<glm::mat4, 128>& jointMatrices,size_t& updatedIndex)
	{
		glm::mat4 m = getMatrix();

		//ボーンの行列の更新
		glm::mat4 inverseTransform = glm::inverse(m);
		size_t numJoints = std::min((uint32_t)skin->joints.size(), 128u);
		if (numJoints <= updatedIndex)
		{
			return;
		}

		for (size_t i = 0; i < numJoints; i++)
		{
			GltfNode* jointNode = skin->joints[i];

			glm::mat4 jointMat = jointNode->getMatrix() * skin->inverseBindMatrices[i];
			jointMat = inverseTransform * jointMat;
			jointMatrices[i] = jointMat;
		}

		updatedIndex = numJoints;
	}
};

struct AnimationChannel {
	enum PathType { TRANSLATION, ROTATION, SCALE };
	PathType path;
	GltfNode* node;
	uint32_t samplerIndex;
};

struct AnimationSampler {
	enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
	InterpolationType interpolation;
	std::vector<float> inputs;
	std::vector<glm::vec4> outputsVec4;
	std::vector<float> outputs;
	glm::vec4 cubicSplineInterpolation(size_t index, float time, uint32_t stride)
	{
		float delta = inputs[index + 1] - inputs[index];
		float t = (time - inputs[index]) / delta;
		const size_t current = index * stride * 3;
		const size_t next = (index + 1) * stride * 3;
		const size_t A = 0;
		const size_t V = stride * 1;
		const size_t B = stride * 2;

		float t2 = powf(t, 2);
		float t3 = powf(t, 3);
		glm::vec4 pt{ 0.0f };
		for (uint32_t i = 0; i < stride; i++) {
			float p0 = outputs[current + i + V];			// starting point at t = 0
			float m0 = delta * outputs[current + i + A];	// scaled starting tangent at t = 0
			float p1 = outputs[next + i + V];				// ending point at t = 1
			float m1 = delta * outputs[next + i + B];		// scaled ending tangent at t = 1
			pt[i] = ((2.f * t3 - 3.f * t2 + 1.f) * p0) + ((t3 - 2.f * t2 + t) * m0) + ((-2.f * t3 + 3.f * t2) * p1) + ((t3 - t2) * m0);
		}
		return pt;
	}

	void translate(size_t index, float time, GltfNode* node)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
			node->translation = glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
			break;
		}
		case AnimationSampler::InterpolationType::STEP: {
			node->translation = outputsVec4[index];
			break;
		}
		case AnimationSampler::InterpolationType::CUBICSPLINE: {
			node->translation = cubicSplineInterpolation(index, time, 3);
			break;
		}
		}
	}

	void scale(size_t index, float time, GltfNode* node)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
			node->scale = glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
			break;
		}
		case AnimationSampler::InterpolationType::STEP: {
			node->scale = outputsVec4[index];
			break;
		}
		case AnimationSampler::InterpolationType::CUBICSPLINE: {
			node->scale = cubicSplineInterpolation(index, time, 3);
			break;
		}
		}
	}

	void rotate(size_t index, float time, GltfNode* node)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
			glm::quat q1;
			q1.x = outputsVec4[index].x;
			q1.y = outputsVec4[index].y;
			q1.z = outputsVec4[index].z;
			q1.w = outputsVec4[index].w;
			glm::quat q2;
			q2.x = outputsVec4[index + 1].x;
			q2.y = outputsVec4[index + 1].y;
			q2.z = outputsVec4[index + 1].z;
			q2.w = outputsVec4[index + 1].w;
			node->rotation = glm::normalize(glm::slerp(q1, q2, u));
			break;
		}
		case AnimationSampler::InterpolationType::STEP: {
			glm::quat q1;
			q1.x = outputsVec4[index].x;
			q1.y = outputsVec4[index].y;
			q1.z = outputsVec4[index].z;
			q1.w = outputsVec4[index].w;
			node->rotation = q1;
			break;
		}
		case AnimationSampler::InterpolationType::CUBICSPLINE: {
			glm::vec4 rot = cubicSplineInterpolation(index, time, 4);
			glm::quat q;
			q.x = rot.x;
			q.y = rot.y;
			q.z = rot.z;
			q.w = rot.w;
			node->rotation = glm::normalize(q);
			break;
		}
		}
	}
};

struct Animation {
	std::string name;
	std::vector<AnimationSampler> samplers;
	std::vector<AnimationChannel> channels;
	float start = std::numeric_limits<float>::max();
	float end = std::numeric_limits<float>::min();

	std::map<float, std::pair<glm::vec3,glm::vec3>> coliderVertices;
};

class GltfModel
{
private:
	GltfNode* root;

public:

	GltfModel(GltfNode* rootNode) 
	{
		this->root = rootNode; 
		this->meshCount = 0;
		this->primitiveCount = 0;
		this->setup = false;
	}
	~GltfModel();
	void deleteNodes(GltfNode* node);

	bool setup;
	int meshCount;
	int primitiveCount;
	int jointNum;
	std::unordered_map<std::string,Animation> animations;
	std::vector<Skin*> skins;

	BoundingBox boundingBox;
	glm::vec3 initPoseMin, initPoseMax;

	GltfNode* getRootNode() { return root; }
	GltfNode* nodeFromIndex(int index);
	GltfNode* findNode(GltfNode* parent,int index);

	void calculateBoundingBox(GltfNode* node,GltfNode* parent);
	void getVertexMinMax(GltfNode* node);
	
	void updateAllNodes(GltfNode* parent, std::vector<std::array<glm::mat4, 128>>& jointMatrices,size_t& updatedIndex);

	float animationDuration(std::string animationName);
	void updateAnimation(float animationTime, Animation& animation, std::vector<std::array<glm::mat4, 128>>& jointMatrices);
	std::map<float, std::pair<glm::vec3, glm::vec3>>& getAnimationAABB(std::string animationName);

	void cleanUpVulkan(VkDevice& device);
	void cleanUpVulkan(VkDevice& device, GltfNode* node);
};