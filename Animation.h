#pragma once

#include<array>
#include<vector>
#include<map>
#include<memory>
#include <assimp/scene.h>           // Output data structure
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Model;

struct AnimationKeyData
{
	std::map<float, glm::vec3> animationScaleKey;
	std::map<float, aiQuaternion> animationRotKey;
	std::map<float, glm::vec3> animationPositionKey;
};

class AnimNode
{
private:
	bool animNode;

	uint32_t childrenCount;
	std::vector<AnimNode*> children;

	std::string name;
	AnimationKeyData animKeyData;
	glm::mat4 matrix;

public:

	AnimNode(std::string nodeName,AnimationKeyData data, unsigned int cCount)
	{
		animNode = true;
		name = nodeName;
		animKeyData = data;
		children.resize(cCount);
		childrenCount = cCount;
	}

	AnimNode(std::string nodeName, glm::mat4 mat,unsigned int cCount)
	{
		animNode = false;
		name = nodeName;
		matrix = mat;
		childrenCount = cCount;
	}

	std::string getNodeName()
	{
		return name;
	}

	glm::mat4 aiMatrix3x3ToGlm(const aiMatrix3x3& from)
	{
		return glm::transpose(glm::make_mat3(&from.a1));
	}

	void resizeChildren(int childNum)
	{
		children.resize(childNum);
	}

	void setChild(int i, AnimNode* child)
	{
		if (i < children.size())
		{
			children[i] = child;
		}
	}

	int getChildrenCount()
	{
		return children.size();
	}

	std::string getName()
	{
		return name;
	}

	AnimNode* getChildren(int i)
	{
		if (i < children.size())
		{
			return children[i];
		}

		return nullptr;
	}

	void findScale(float animTime,float& t1,float& t2)
	{
		auto end = animKeyData.animationScaleKey.end();
		end--;

		for (auto itr = animKeyData.animationScaleKey.begin(); itr != end; itr++)
		{
			auto next = itr;
			next++;

			if (animTime < next->first)
			{
				t1 = itr->first;
				t2 = next->first;
				return;
			}
		}

		t1 = -1.0f;
		t2 = -1.0f;
	}

	void findRotate(float animTime, float& t1, float& t2)
	{
		auto end = animKeyData.animationRotKey.end();
		end--;

		for (auto itr = animKeyData.animationRotKey.begin(); itr != end; itr++)
		{
			auto next = itr;
			next++;

			if (animTime < next->first)
			{
				t1 = itr->first;
				t2 = next->first;
				return;
			}
		}

		t1 = -1.0f;
		t2 = -1.0f;
	}

	void findPosition(float animTime, float& t1, float& t2)
	{
		auto end = animKeyData.animationPositionKey.end();
		end--;

		for (auto itr = animKeyData.animationPositionKey.begin(); itr != end; itr++)
		{
			auto next = itr;
			next++;

			if (animTime < next->first)
			{
				t1 = itr->first;
				t2 = next->first;
				return;
			}
		}

		t1 = -1.0f;
		t2 = -1.0f;
	}

	glm::vec3 calcInterpolatedScaling(float animTime)
	{
		if (animKeyData.animationScaleKey.size() == 1)
		{
			return animKeyData.animationScaleKey.begin()->second;
		}

		float t1, t2;

		findScale(animTime,t1,t2);//現在の時刻に最も近いノードを探す

		if (t1 >= 0.0f)
		{

			float DeltaTime = t2 - t1;//時間の差分を取得
			float Factor = (animTime - t1) / DeltaTime;//時間の差分をとる、現在の時間と直前のノードの時間を取得して、直前のノードと直後のノードの間のどれくらいの場所にいるのかを調べる

			const glm::vec3 Start = animKeyData.animationScaleKey[t1];//スケーリングの値をとる
			const glm::vec3 End = animKeyData.animationScaleKey[t2];//スケーリングの値をとる
			glm::vec3 Delta = End - Start;//差分とる
			return Start + Factor * Delta;//比率から計算する
		}
		else//最初のキーフレームよりも前の時間
		{

			float t2 = animKeyData.animationScaleKey.begin()->first;
			float Factor = 1 - (t2 - animTime) / t2;

			if (!(Factor >= 0.0f && Factor <= 1.0f)) {
				Factor = 1.f;
			}

			const glm::vec3 Start = glm::vec3(1.0, 1.0, 1.0);
			const glm::vec3 End = animKeyData.animationScaleKey.begin()->second;
			glm::vec3 Delta = End - Start;

			return Start + Factor * Delta;
		}
	}

	aiQuaternion calcInterpolatedQuat(float animTime)
	{
		// 補間には最低でも２つの値が必要
		if (animKeyData.animationRotKey.size() == 1) {
			return animKeyData.animationRotKey.begin()->second;
		}

		float t1, t2;

		findRotate(animTime,t1,t2);
		
		if (t1 >= 0.0f)
		{
			float DeltaTime = t2 - t1;
			float Factor = (animTime - t1) / DeltaTime;

			const aiQuaternion StartRotationQ = animKeyData.animationRotKey[t1];
			const aiQuaternion EndRotationQ = animKeyData.animationRotKey[t2];

			aiQuaternion Out;
			aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
			Out = Out.Normalize();
			return Out;
		}
		else//最初のキーフレームよりも前の時間
		{
			float t2 = animKeyData.animationRotKey.begin()->first;
			float Factor = 1 - (t2 - animTime) / t2;

			if (!(Factor >= 0.0f && Factor <= 1.0f)) {
				Factor = 1.f;
			}

			const aiQuaternion& StartRotationQ = aiQuaternion(1.0,0.0,0.0,0.0);
			const aiQuaternion& EndRotationQ = animKeyData.animationRotKey.begin()->second;
			aiQuaternion Out;
			aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
			Out = Out.Normalize();
			return Out;
		}
	}

	glm::vec3 calcInterpolatedPos(float animTime)
	{
		if (animKeyData.animationPositionKey.size() == 1)
		{
			return animKeyData.animationPositionKey.begin()->second;
		}

		float t1, t2;

		findPosition(animTime,t1,t2);//現在の時刻に最も近いノードを探す
		
		if (t1 >= 0.0f)
		{
			float DeltaTime = t2 - t1;//時間の差分を取得
			float Factor = (animTime - t1) / DeltaTime;//時間の差分をとる、現在の時間と直前のノードの時間を取得して、直前のノードと直後のノードの間のどれくらいの場所にいるのかを調べる

			const glm::vec3 Start = animKeyData.animationPositionKey[t1];//スケーリングの値をとる
			const glm::vec3 End = animKeyData.animationPositionKey[t2];//スケーリングの値をとる
			glm::vec3 Delta = End - Start;//差分とる
			return Start + Factor * Delta;//比率から計算する
		}
		else//最初のキーフレームよりも前の時間
		{
			float t2 = animKeyData.animationPositionKey.begin()->first;
			float Factor = 1 - (t2 - animTime) / t2;
			
			if (!(Factor >= 0.0f && Factor <= 1.0f)) {
				Factor = 1.f;
			}

			const glm::vec3 Start = glm::vec3(0.0, 0.0, 0.0);
			const glm::vec3 End = animKeyData.animationPositionKey.begin()->second;
			glm::vec3 Delta = End - Start;
			return Start + Factor * Delta;
		}
	}

	glm::mat4 getAnimMatrix(float animTime,glm::mat4& transform)
	{
		//補間などを計算して、平行移動と回転、スケーリングの行列を合成した行列を返す
		if (animNode)
		{
			glm::vec3 scale = calcInterpolatedScaling(animTime);
			glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f),scale);

			aiQuaternion quat = calcInterpolatedQuat(animTime);
			glm::mat4 quatMat = aiMatrix3x3ToGlm(quat.GetMatrix());


			glm::vec3 position = calcInterpolatedPos(animTime);
			glm::mat4 posMat = glm::translate(glm::mat4(1.0f), position);

			return transform * posMat * quatMat * scaleMat;
		}
		else
		{
			return transform * matrix;
		}
	}
};

struct BoneInfo
{
	std::vector<glm::mat4> offsetMatrix;
};

class FbxModel;

class Pose
{
private:
	std::array<glm::mat4, 250> boneMatrix;

public:
	void setPoseMatrix(std::array<glm::mat4, 250>& matrix);

	void setFinalTransform(std::array<glm::mat4, 250>& boneFinalTransforms);
};

struct VertexMorphInfo
{
	std::array<unsigned int, 3> values;
	std::array<float, 3> weights;
};

class Animation
{
private:

	float timeTick;
	float duration;

	AnimNode* rootNode;

	glm::mat4 inverseGlobalTransform;

public:

	Animation();
	Animation(float timeInTick,float duration);
	~Animation();
	void DeleteAnimTree(AnimNode* node);

	void setRootNode(AnimNode* rootNode)
	{
		this->rootNode = rootNode;
	}

	void setGlobalInverseTransform(glm::mat4 mat) { inverseGlobalTransform = mat; }
	void setFinalTransform(float animationTime, std::array<glm::mat4, 250>& boneFinalTransforms, FbxModel* model);
	void setFinalTransform(float animationTime, std::array<glm::mat4, 250>& boneFinalTransforms,AnimNode* node,glm::mat4 parentMatrix, FbxModel* model);
};