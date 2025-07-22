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

//AABB用の構造体
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

//頂点をまとめる用のプリミティブ
//プリミティブ単位でマテリアルを指定される
struct Primitive
{
	int primitiveIndex;//番号
	int firstIndex;//インデックスバッファ内のこのプリミティブのインデックスの開始位置
	int indexCount;//プリミティブのインデックスの数
	int vertexCount;//頂点の数
	int materialIndex;//このプリミティブが持つマテリアルの番号
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

//スケルトンの構造体
struct Skin
{
	std::string name;
	GltfNode* skeletonRoot = nullptr;
	std::vector<glm::mat4> inverseBindMatrices;
	std::vector<GltfNode*> joints;
};

//プリミティブをひとまとめにしたもの
struct Mesh
{
	//Meshの持つ頂点をバッファから参照するためのindex
	uint32_t meshIndex;
	std::vector<Vertex> vertices;//頂点配列
	std::vector<uint32_t> indices;//インデックス配列
	std::vector<Primitive> primitives;

	glm::vec3 min;
	glm::vec3 max;
};

//ノードごとのトランスフォームを記録
struct NodeTransform
{
	std::vector<glm::vec3> translation;
	std::vector<glm::quat> rotation;
	std::vector<glm::vec3> scale;

	std::vector<glm::mat4> matrix;
	std::vector<glm::mat4> nodeTransform;

	void setNodeCount(int nodeCount)
	{
		translation.resize(nodeCount);
		rotation.resize(nodeCount);
		scale.resize(nodeCount);
		matrix.resize(nodeCount);
		nodeTransform.resize(nodeCount);

		std::fill(translation.begin(), translation.end(), glm::vec3(0.0f));
		std::fill(rotation.begin(), rotation.end(), glm::quat());
		std::fill(scale.begin(), scale.end(), glm::vec3(1.0f));
		std::fill(matrix.begin(), matrix.end(), glm::mat4(1.0f));
		std::fill(nodeTransform.begin(), nodeTransform.end(), glm::mat4(1.0f));
	}
};

struct GltfNode
{
	uint32_t index;
	std::vector<GltfNode*> children;
	glm::mat4 matrix;//ローカル空間へ変換用行列
	std::string name;
	std::vector<Mesh*> meshArray;
	Skin* skin;
	int skinIndex = -1;
	int globalHasSkinNodeIndex = 0;
	int firstIndex;
	int indexCount;

	DescriptorInfo descriptorInfo;

	BoundingBox bvh;
	
	GltfNode()
	{
		matrix = glm::mat4(1.0f);
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

	void setLocalMatrix(NodeTransform& nodeTransform, glm::mat4 parentMatrix)
	{
		//このノード個別の行列
		glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), nodeTransform.translation[index]) *
			glm::mat4(nodeTransform.rotation[index]) * glm::scale(glm::mat4(1.0f), nodeTransform.scale[index]) * matrix;

		nodeTransform.nodeTransform[index] = parentMatrix * localMatrix;

		nodeTransform.matrix[index] = matrix;

		for (auto& child : children)
		{
			child->setLocalMatrix(nodeTransform, nodeTransform.nodeTransform[index]);
		}
	}

	void setLocalMatrix(NodeTransform& nodeTransform)
	{
		//このノード個別の行列
		glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), nodeTransform.translation[index]) *
			glm::mat4(nodeTransform.rotation[index]) * glm::scale(glm::mat4(1.0f), nodeTransform.scale[index]) * matrix;

		nodeTransform.nodeTransform[index] = localMatrix;

		nodeTransform.matrix[index] = matrix;

		for (auto& child : children)
		{
			child->setLocalMatrix(nodeTransform, nodeTransform.nodeTransform[index]);
		}
	}

	//このノードが所属するスケルトンのアニメーション行列を計算する
	void update(NodeTransform& nodeTransform, std::array<glm::mat4, 128>& jointMatrices, size_t& updatedIndex)
	{
		glm::mat4 m = nodeTransform.nodeTransform[index];

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

			glm::mat4 jointMat = nodeTransform.nodeTransform[jointNode->index] * skin->inverseBindMatrices[i];
			jointMat = inverseTransform * jointMat;
			jointMatrices[i] = jointMat;
		}

		//計算し終えたジョイントを記録して、メモ化を行い、同じジョイントの行列を計算しない
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

//アニメーションのキーの持つ行列が移動、回転、拡大のどれに関するか示す
struct AnimationChannel {
	enum PathType { TRANSLATION, ROTATION, SCALE };
	PathType path;
	GltfNode* node;
	uint32_t samplerIndex;
};

//アニメーションの保管に関する構造体
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

	//平行移動の補間
	glm::vec3 translate(size_t index, double time, GltfNode* node)
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

	//拡大の補間
	glm::vec3 scale(size_t index, double time, GltfNode* node)
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

	//回転の補間
	glm::quat rotate(size_t index, double time, GltfNode* node)
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

//アニメーション全体の構造体
struct Animation {
	std::string name;
	std::vector<AnimationSampler> samplers;
	std::vector<AnimationChannel> channels;
	float start = std::numeric_limits<float>::max();//アニメーションの開始時間
	float end = std::numeric_limits<float>::min();//終了時間
};

//読み込んだgltfモデル全体のクラス
class GltfModel
{
private:
	GltfNode* root;

	//gltfモデルの頂点関連用のバッファ
	std::vector<std::shared_ptr<GpuBuffer>> vertBuffer;
	std::vector<std::shared_ptr<GpuBuffer>> indeBuffer;

	//ディスクリプタセット
	std::vector<std::shared_ptr<DescriptorSet>> descriptorSet;

	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	void createBuffer(const GltfNode* node);
	void createDescriptorSet(const GltfNode* node
		, std::vector<std::shared_ptr<DescriptorSet>>& descriptorSet);

public:

	GltfModel(GltfNode* rootNode 
		,std::shared_ptr<GpuBufferFactory> bf
		,std::shared_ptr<DescriptorSetLayoutFactory> layout
		,std::shared_ptr<DescriptorSetFactory> desc)
	{
		this->root = rootNode; 
		this->meshCount = 0;
		this->primitiveCount = 0;
		this->setup = false;
		this->jointNum = 0;

		bufferFactory = bf;
		layoutFactory = layout;
		descriptorSetFactory = desc;
	}
	~GltfModel();

	void setPointBufferNum();

	void deleteNodes(GltfNode* node);

	bool setup;
	
	//メッシュの数、プリミティブの数、ジョイントの数、Modelクラスに設定した際の、バッファの作成時に利用
	int meshCount;
	int primitiveCount;
	int jointNum;
	int vertexNum;
	int indexNum;

	//ノードの個数
	int nodeCount;

	//アニメーションの名前をキーとして、アニメーションを記録
	std::unordered_map<std::string,Animation> animations;
	//スケルトン 通常は一つ
	std::vector<Skin*> skins;

	//同じくマテリアルデータ Primitive構造体のmaterialIndexから指定される
	std::vector<std::shared_ptr<Material>> materials;

	BoundingBox boundingBox;
	glm::vec3 initPoseMin, initPoseMax;

	GltfNode* getRootNode() { return root; }
	GltfNode* nodeFromIndex(int index);
	GltfNode* findNode(GltfNode* parent,int index);

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

	//AABBの計算
	void calculateBoundingBox(GltfNode* node,GltfNode* parent);
	//gltfモデルの初期ポーズの頂点の座標の最小値最大値の取得
	void getVertexMinMax(GltfNode* node);
	
	//アニメーションの各ノードの更新処理
	void updateAllNodes(GltfNode* parent, NodeTransform& nodeTransform
		, std::vector<std::array<glm::mat4, 128>>& jointMatrices, size_t& updatedIndex);

	//アニメーションの長さを取得
	float animationDuration(std::string animationName);
	//指定したアニメーションの行列を取得
	void updateAnimation(double animationTime, std::string animationName, NodeTransform& nodeTransform
		, std::vector<std::array<glm::mat4, 128>>& jointMatrices);
};