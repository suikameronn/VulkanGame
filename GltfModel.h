#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include<iostream>
#include<vector>
#include<array>
#include<map>
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include"Material.h"
#include"StructList.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

struct Vertex;
struct Primitive;
struct Mesh;
struct Skin;
struct GltfNode;
struct Animation;
struct BoundingBox;

//AABB�p�̍\����
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

//���_���܂Ƃ߂�p�̃v���~�e�B�u
//�v���~�e�B�u�P�ʂŃ}�e���A�����w�肳���
struct Primitive
{
	int primitiveIndex;//�ԍ�
	int firstIndex;//�C���f�b�N�X�o�b�t�@���̂��̃v���~�e�B�u�̃C���f�b�N�X�̊J�n�ʒu
	int indexCount;//�v���~�e�B�u�̃C���f�b�N�X�̐�
	int vertexCount;//���_�̐�
	int materialIndex;//���̃v���~�e�B�u�����}�e���A���̔ԍ�
	bool hasIndices;
	BoundingBox bb;//aabb

	Primitive(int primitiveIndex,int firstIndex, int indexCount, int vertexCount,int materialIndex)
		: primitiveIndex(primitiveIndex),firstIndex(firstIndex), indexCount(indexCount), vertexCount(vertexCount), materialIndex(materialIndex) {
		hasIndices = indexCount > 0;
	}
	void setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}
};

//�X�P���g���̍\����
struct Skin
{
	std::string name;

	//�X�L���̃��[�g�m�[�h�̔z���̈ʒu
	int skinRootNodeOffset;
	std::vector<glm::mat4> inverseBindMatrices;
	std::vector<int> jointNodeOffset;
};

//�v���~�e�B�u���ЂƂ܂Ƃ߂ɂ�������
struct Mesh
{
	//Mesh�̎����_���o�b�t�@����Q�Ƃ��邽�߂�index
	uint32_t meshIndex;
	std::vector<Vertex> vertices;//���_�z��
	std::vector<uint32_t> indices;//�C���f�b�N�X�z��
	std::vector<Primitive> primitives;

	glm::vec3 min;
	glm::vec3 max;
};

//�m�[�h���Ƃ̃g�����X�t�H�[�����L�^
struct NodeTransform
{
	std::vector<glm::vec3> translation;
	std::vector<glm::quat> rotation;
	std::vector<glm::vec3> scale;

	std::vector<glm::mat4> matrix;
	std::vector<glm::mat4> nodeTransform;

	void init()
	{
		translation.clear();
		rotation.clear();
		scale.clear();

		matrix.clear();
		nodeTransform.clear();
	}

	void setNodeCount(int nodeCount)
	{
		translation.resize(nodeCount);
		rotation.resize(nodeCount);
		scale.resize(nodeCount);
		matrix.resize(nodeCount);
		nodeTransform.resize(nodeCount);
	}

	void resetMat()
	{
		std::fill(translation.begin(), translation.end(), glm::vec3(0.0f));
		std::fill(rotation.begin(), rotation.end(), glm::quat());
		std::fill(scale.begin(), scale.end(), glm::vec3(1.0f));
		std::fill(matrix.begin(), matrix.end(), glm::mat4(1.0f));
		std::fill(nodeTransform.begin(), nodeTransform.end(), glm::mat4(1.0f));
	}
};

struct GltfNode
{
	//���̃m�[�h���z�u����Ă���z���̈ʒu
	size_t offset;
	//����index��tinygltf��̔ԍ�
	uint32_t index;
	//parentIndex���ۂ̔z���̐e�m�[�h�̈ʒu
	size_t parentIndex;
	glm::mat4 matrix;//���[�J����Ԃ֕ϊ��p�s��
	std::string name;
	Mesh mesh;
	Skin skin;
	int skinIndex = -1;
	int globalHasSkinNodeIndex = 0;
	int firstIndex;
	int indexCount;

	DescriptorInfo descriptorInfo;

	BoundingBox bvh;
	
	GltfNode()
	{
		offset = 0;
		matrix = glm::mat4(1.0f);
		index = 0;
		parentIndex = -1;
	}

	~GltfNode()
	{
	}

	void setLocalMatrix(NodeTransform& nodeTransform)
	{
		//���̃m�[�h�ʂ̍s��
		glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), nodeTransform.translation[offset]) *
			glm::mat4(nodeTransform.rotation[offset]) * glm::scale(glm::mat4(1.0f), nodeTransform.scale[offset]) * matrix;

		glm::mat4 parentMatrix = glm::mat4(1.0f);

		if (parentIndex > -1)
		{
			parentMatrix = nodeTransform.nodeTransform[parentIndex];
		}

		nodeTransform.nodeTransform[offset] = parentMatrix * localMatrix;

		nodeTransform.matrix[offset] = matrix;
	}

	//���̃m�[�h����������X�P���g���̃A�j���[�V�����s����v�Z����
	void update(NodeTransform& nodeTransform, std::array<glm::mat4, 128>& jointMatrices, size_t& updatedIndex)
	{
		glm::mat4 m = nodeTransform.nodeTransform[offset];

		//�{�[���̍s��̍X�V
		glm::mat4 inverseTransform = glm::inverse(m);
		size_t numJoints = std::min((uint32_t)skin.jointNodeOffset.size(), 128u);
		if (numJoints <= updatedIndex)
		{
			return;
		}

		for (size_t i = 0; i < numJoints; i++)
		{
			glm::mat4 jointMat = nodeTransform.nodeTransform[skin.jointNodeOffset[i]] * skin.inverseBindMatrices[i];
			jointMat = inverseTransform * jointMat;
			jointMatrices[i] = jointMat;
		}

		//�v�Z���I�����W���C���g���L�^���āA���������s���A�����W���C���g�̍s����v�Z���Ȃ�
		updatedIndex = numJoints;
	}

	int getJointCount() const
	{
		if (skin.name.size() == 0)
		{
			return 0;
		}

		return std::min((uint32_t)skin.jointNodeOffset.size(), 128u);
	}
};

//�A�j���[�V�����̃L�[�̎��s�񂪈ړ��A��]�A�g��̂ǂ�Ɋւ��邩����
struct AnimationChannel {
	enum PathType { TRANSLATION, ROTATION, SCALE };
	PathType path;
	size_t nodeOffset;
	uint32_t samplerIndex;
};

//�A�j���[�V�����̕ۊǂɊւ���\����
struct AnimationSampler {
	enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
	InterpolationType interpolation;
	std::vector<float> inputs;
	std::vector<glm::vec4> outputsVec4;
	std::vector<float> outputs;
	glm::vec4 cubicSplineInterpolation(size_t index, double time, uint32_t stride)
	{
		float delta = inputs[index + 1] - inputs[index];
		float t = static_cast<float>((time - inputs[index]) / delta);
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

	//���s�ړ��̕��
	glm::vec3 translate(size_t index, double time)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = static_cast<float>(std::max(0.0, time - inputs[index]) / (inputs[index + 1] - inputs[index]));
			return glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
		}
		case AnimationSampler::InterpolationType::STEP: {
			return outputsVec4[index];
		}
		case AnimationSampler::InterpolationType::CUBICSPLINE: {
			return cubicSplineInterpolation(index, time, 3);
		}
		}
	}

	//�g��̕��
	glm::vec3 scale(size_t index, double time)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = static_cast<float>(std::max(0.0, time - inputs[index]) / (inputs[index + 1] - inputs[index]));
			return glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
		}
		case AnimationSampler::InterpolationType::STEP: {
			return outputsVec4[index];
		}
		case AnimationSampler::InterpolationType::CUBICSPLINE: {
			return cubicSplineInterpolation(index, time, 3);
		}
		}
	}

	//��]�̕��
	glm::quat rotate(size_t index, double time)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = static_cast<float>(std::max(0.0, time - inputs[index]) / (inputs[index + 1] - inputs[index]));
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
			return glm::normalize(glm::slerp(q1, q2, u));
		}
		case AnimationSampler::InterpolationType::STEP: {
			glm::quat q1;
			q1.x = outputsVec4[index].x;
			q1.y = outputsVec4[index].y;
			q1.z = outputsVec4[index].z;
			q1.w = outputsVec4[index].w;
			return q1;
		}
		case AnimationSampler::InterpolationType::CUBICSPLINE: {
			glm::vec4 rot = cubicSplineInterpolation(index, time, 4);
			glm::quat q;
			q.x = rot.x;
			q.y = rot.y;
			q.z = rot.z;
			q.w = rot.w;
			return glm::normalize(q);
		}
		}
	}
};

//�A�j���[�V�����S�̂̍\����
struct Animation {
	std::string name;
	std::vector<AnimationSampler> samplers;
	std::vector<AnimationChannel> channels;
	float start = std::numeric_limits<float>::max();//�A�j���[�V�����̊J�n����
	float end = std::numeric_limits<float>::min();//�I������
};

//�ǂݍ���gltf���f���S�̂̃N���X
class GltfModel
{
private:

	//gltf���f���̒��_�֘A�p�̃o�b�t�@
	std::vector<std::shared_ptr<GpuBuffer>> vertBuffer;
	std::vector<std::shared_ptr<GpuBuffer>> indeBuffer;

	//�f�B�X�N���v�^�Z�b�g
	std::vector<std::shared_ptr<DescriptorSet>> descriptorSet;

	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

public:

	GltfModel(std::shared_ptr<GpuBufferFactory> bf
		,std::shared_ptr<DescriptorSetLayoutFactory> layout
		,std::shared_ptr<DescriptorSetFactory> desc)
	{
		this->meshCount = 0;
		this->primitiveCount = 0;
		this->setup = false;
		this->jointNum = 0;

		bufferFactory = bf;
		layoutFactory = layout;
		descriptorSetFactory = desc;
	}
	~GltfModel();

	std::vector<GltfNode> nodes;

	void setPointBufferNum();

	bool setup;
	
	//���b�V���̐��A�v���~�e�B�u�̐��A�W���C���g�̐��AModel�N���X�ɐݒ肵���ۂ́A�o�b�t�@�̍쐬���ɗ��p
	int meshCount;
	int primitiveCount;
	int jointNum;
	int vertexNum;
	int indexNum;

	//�m�[�h�̌�
	int nodeCount;

	//�A�j���[�V�����̖��O���L�[�Ƃ��āA�A�j���[�V�������L�^
	std::unordered_map<std::string,Animation> animations;
	//�X�P���g�� �ʏ�͈��
	std::vector<Skin> skins;

	//�������}�e���A���f�[�^ Primitive�\���̂�materialIndex����w�肳���
	std::vector<std::shared_ptr<Material>> materials;

	BoundingBox boundingBox;
	glm::vec3 initPoseMin, initPoseMax;

	int nodeFromIndex(const int& index);
	int findNode(const int& index);

	void createBuffer();
	void createDescriptorSet(std::vector<std::shared_ptr<DescriptorSet>>& descriptorSet);

	std::shared_ptr<GpuBuffer> getVertBuffer(int index)
	{
		return vertBuffer[index];
	}

	std::shared_ptr<GpuBuffer> getIndeBuffer(int index)
	{
		return indeBuffer[index];
	}

	//�A�j���[�V�����̊e�m�[�h�̍X�V����
	void updateAllNodes(NodeTransform& nodeTransform
		, std::vector<std::array<glm::mat4, 128>>& jointMatrices, size_t& updatedIndex);

	//�A�j���[�V�����̒������擾
	float animationDuration(std::string animationName);
	//�w�肵���A�j���[�V�����̍s����擾
	void updateAnimation(double animationTime, std::string animationName, NodeTransform& nodeTransform
		, std::vector<std::array<glm::mat4, 128>>& jointMatrices);
};