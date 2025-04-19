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

//���_�f�[�^
struct Vertex {
	glm::vec3 pos;//���W
	glm::vec3 color;//���_�J���[
	glm::vec2 texCoord0;//uv���W
	glm::vec2 texCoord1;
	glm::vec3 normal;//�@��

	uint32_t index;
	glm::ivec4 boneID1;//�e���󂯂�W���C���g�̔ԍ�

	glm::vec4 weight1;//�W���C���g�̉e���x

	Vertex()
	{
		pos = glm::vec3(0.0);
		color = glm::vec3(0.0);
		texCoord0 = glm::vec2(0.0);
		texCoord1 = glm::vec2(0.0);
		normal = glm::vec3(0.0);

		index = 0;

		for (uint32_t i = 0; i < 4; i++)
		{
			boneID1[i] = 0;
			weight1[i] = 0.0f;
		}
	};

	//�W���C���g�Ɖe���x��ݒ�
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
	GltfNode* skeletonRoot = nullptr;
	std::vector<glm::mat4> inverseBindMatrices;
	std::vector<GltfNode*> joints;
};

//�v���~�e�B�u���ЂƂ܂Ƃ߂ɂ�������
struct Mesh
{
	//Mesh�̎����_���o�b�t�@����Q�Ƃ��邽�߂�index
	uint32_t meshIndex;
	std::vector<Vertex> vertices;//���_�z��
	std::vector<uint32_t> indices;//�C���f�b�N�X�z��
	std::vector<Primitive> primitives;

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
	glm::mat4 matrix;//���[�J����Ԃ֕ϊ��p�s��
	std::string name;
	std::vector<Mesh*> meshArray;
	Skin* skin;
	int skinIndex = -1;
	int globalHasSkinNodeIndex = 0;
	glm::vec3 translation{};
	glm::vec3 scale{ 1.0f };
	glm::quat rotation{};

	DescriptorInfo descriptorInfo;

	BoundingBox bvh;
	BoundingBox aabb;
	
	GltfNode()
	{
		matrix = glm::mat4(1.0f);
		parent = nullptr;
		index = 0;
		std::fill(meshArray.begin(),meshArray.end(),nullptr);
		skin = nullptr;
	}

	~GltfNode()
	{
		for (auto mesh : meshArray)
		{
			delete mesh;
		}
	}

	glm::mat4 localMatrix()//���̃m�[�h�̃��[�J����Ԃւ̕ϊ��s��̌v�Z
	{
		return glm::translate(glm::mat4(1.0f), translation) * 
			glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
	}

	glm::mat4 getMatrix()//�ċA�I�ɂ��̃m�[�h�܂ł̕ϊ��s������߂�
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

	//���̃m�[�h����������X�P���g���̃A�j���[�V�����s����v�Z����
	void update(std::array<glm::mat4, 128>& jointMatrices,size_t& updatedIndex)
	{
		glm::mat4 m = getMatrix();

		//�{�[���̍s��̍X�V
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

		//�v�Z���I�����W���C���g���L�^���āA���������s���A�����W���C���g�̍s����v�Z���Ȃ�
		updatedIndex = numJoints;
	}

	int getJointCount()
	{
		if (!skin)
		{
			return 0;
		}

		return std::min((uint32_t)skin->joints.size(), 128u);
	}
};

//�A�j���[�V�����̃L�[�̎��s�񂪈ړ��A��]�A�g��̂ǂ�Ɋւ��邩����
struct AnimationChannel {
	enum PathType { TRANSLATION, ROTATION, SCALE };
	PathType path;
	GltfNode* node;
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
	void translate(size_t index, double time, GltfNode* node)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = static_cast<float>(std::max(0.0, time - inputs[index]) / (inputs[index + 1] - inputs[index]));
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

	//�g��̕��
	void scale(size_t index, double time, GltfNode* node)
	{
		switch (interpolation) {
		case AnimationSampler::InterpolationType::LINEAR: {
			float u = static_cast<float>(std::max(0.0, time - inputs[index]) / (inputs[index + 1] - inputs[index]));
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

	//��]�̕��
	void rotate(size_t index, double time, GltfNode* node)
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

	void deleteNodes(GltfNode* node,VkDevice& device);

	bool setup;
	
	//���b�V���̐��A�v���~�e�B�u�̐��A�W���C���g�̐��AModel�N���X�ɐݒ肵���ۂ́A�o�b�t�@�̍쐬���ɗ��p
	int meshCount;
	int primitiveCount;
	int jointNum;
	int vertexNum;
	int indexNum;

	//�A�j���[�V�����̖��O���L�[�Ƃ��āA�A�j���[�V�������L�^
	std::unordered_map<std::string,Animation> animations;
	//�X�P���g�� �ʏ�͈��
	std::vector<Skin*> skins;

	//gltf���f���Ŏg����摜�f�[�^
	std::vector < std::shared_ptr<ImageData>> imageDatas;
	//�������e�N�X�`���f�[�^
	std::vector<TextureData*> textureDatas;
	//�������}�e���A���f�[�^ Primitive�\���̂�materialIndex����w�肳���
	std::vector<std::shared_ptr<Material>> materials;

	BoundingBox boundingBox;
	glm::vec3 initPoseMin, initPoseMax;

	GltfNode* getRootNode() { return root; }
	GltfNode* nodeFromIndex(int index);
	GltfNode* findNode(GltfNode* parent,int index);

	//AABB�̌v�Z
	void calculateBoundingBox(GltfNode* node,GltfNode* parent);
	//gltf���f���̏����|�[�Y�̒��_�̍��W�̍ŏ��l�ő�l�̎擾
	void getVertexMinMax(GltfNode* node);
	
	//�A�j���[�V�����̊e�m�[�h�̍X�V����
	void updateAllNodes(GltfNode* parent, std::vector<std::array<glm::mat4, 128>>& jointMatrices,size_t& updatedIndex);

	//�A�j���[�V�����̒������擾
	float animationDuration(std::string animationName);
	//�w�肵���A�j���[�V�����̍s����擾
	void updateAnimation(double animationTime, Animation& animation, std::vector<std::array<glm::mat4, 128>>& jointMatrices);
	//gpu��̃o�b�t�@�Ȃǂ̍폜����
	void cleanUpVulkan(VkDevice& device);
};