#pragma once

#include<vector>
#include<unordered_map>
#include<memory>
#include <assimp/scene.h>           // Output data structure
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

struct AnimationKeyData
{
	std::unordered_map<float, glm::vec3> animationScaleKey;
	std::unordered_map<float, aiQuaternion> animationRotKey;
	std::unordered_map<float, glm::vec3> animationPositionKey;
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
	}

	AnimNode(std::string nodeName, glm::mat4 mat,unsigned int cCount)
	{
		animNode = false;
		name = nodeName;
		matrix = mat;
	}

	std::string getNodeName()
	{
		return name;
	}

	glm::mat4 aiMatrix3x3ToGlm(const aiMatrix3x3& from)
	{
		glm::mat4 to;

		to[0][0] = from.a1; to[0][1] = from.b1;  to[0][2] = from.c1;
		to[1][0] = from.a2; to[1][1] = from.b2;  to[1][2] = from.c2;
		to[2][0] = from.a3; to[2][1] = from.b3;  to[2][2] = from.c3;

		return to;
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

	std::unordered_map<float, glm::vec3>::iterator findScale(float animTime)
	{
		for (auto itr = animKeyData.animationScaleKey.begin(); itr != animKeyData.animationScaleKey.end(); itr++)
		{
			if (itr->first <= animTime)
			{
				return itr;
			}
		}
	}

	std::unordered_map<float, aiQuaternion>::iterator findRotate(float animTime)
	{
		for (auto itr = animKeyData.animationRotKey.begin(); itr != animKeyData.animationRotKey.end(); itr++)
		{
			if (itr->first <= animTime)
			{
				return itr;
			}
		}
	}

	std::unordered_map<float, glm::vec3>::iterator findPosition(float animTime)
	{
		for (auto itr = animKeyData.animationPositionKey.begin(); itr != animKeyData.animationPositionKey.end(); itr++)
		{
			if (itr->first <= animTime)
			{
				return itr;
			}
		}
	}

	glm::vec3 calcInterpolatedScaling(float animTime)
	{
		if (animKeyData.animationScaleKey.size() == 1)
		{
			return animKeyData.animationScaleKey.begin()->second;
		}

		auto ScalingItr = findScale(animTime);//現在の時刻に最も近いノードを探す
		auto NextScalingItr = ScalingItr++;//そのノードの一つ後のノードを探す

		float t1 = ScalingItr->first;//そのノードの時間を取得
		float t2 = NextScalingItr->first;
		float DeltaTime = t2 - t1;//時間の差分を取得
		float Factor = (animTime - t1) / DeltaTime;//時間の差分をとる、現在の時間と直前のノードの時間を取得して、直前のノードと直後のノードの間のどれくらいの場所にいるのかを調べる

		const glm::vec3 Start = ScalingItr->second;//スケーリングの値をとる
		const glm::vec3 End = NextScalingItr->second;//スケーリングの値をとる
		glm::vec3 Delta = End - Start;//差分とる
		return Start + Factor * Delta;//比率から計算する
	}

	aiQuaternion calcInterpolatedQuat(float animTime)
	{
		// 補間には最低でも２つの値が必要
		if (animKeyData.animationRotKey.size() == 1) {
			return animKeyData.animationRotKey.begin()->second;
		}

		auto RotationItr = findRotate(animTime);
		auto NextRotationItr = RotationItr++;

		float DeltaTime = NextRotationItr->first - RotationItr->first;
		float Factor = (animTime - RotationItr->first) / DeltaTime;

		const aiQuaternion& StartRotationQ = RotationItr->second;
		const aiQuaternion& EndRotationQ = NextRotationItr->second;
		
		aiQuaternion Out;
		aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
		Out = Out.Normalize();

		return Out;
	}

	glm::vec3 calcInterpolatedPos(float animTime)
	{
		if (animKeyData.animationPositionKey.size() == 1)
		{
			return animKeyData.animationPositionKey.begin()->second;
		}

		auto PositionItr = findPosition(animTime);//現在の時刻に最も近いノードを探す
		auto NextPositionItr = PositionItr++;//そのノードの一つ後のノードを探す

		float t1 = PositionItr->first;//そのノードの時間を取得
		float t2 = NextPositionItr->first;
		float DeltaTime = t2 - t1;//時間の差分を取得
		float Factor = (animTime - t1) / DeltaTime;//時間の差分をとる、現在の時間と直前のノードの時間を取得して、直前のノードと直後のノードの間のどれくらいの場所にいるのかを調べる

		const glm::vec3 Start = PositionItr->second;//スケーリングの値をとる
		const glm::vec3 End = NextPositionItr->second;//スケーリングの値をとる
		glm::vec3 Delta = End - Start;//差分とる
		return Start + Factor * Delta;//比率から計算する
	}

	glm::mat4 getAnimMatrix(float animTime)
	{
		glm::mat4 transform;

		//補間などを計算して、平行移動と回転、スケーリングの行列を合成した行列を返す
		if (animNode)
		{
			glm::vec3 scale = calcInterpolatedScaling(animTime);
			glm::mat4 scaleMat = glm::scale(glm::mat4(),scale);

			aiQuaternion quat = calcInterpolatedQuat(animTime);
			glm::mat4 quatMat = aiMatrix3x3ToGlm(quat.GetMatrix());

			glm::vec3 position = calcInterpolatedPos(animTime);
			glm::mat4 posMat = glm::translate(glm::mat4(), position);

			transform = scaleMat * quatMat * posMat;
		}
		else
		{
			transform = matrix;
		}

		return transform;
	}
};

class Animation
{
private:
	bool play = false;

	float timeTick;
	float duration;
	float timeSeconds;
	float animationTime;

	std::vector<std::vector<glm::mat4>> transforms;

	AnimNode* rootNode;

public:

	Animation(float timeInTick,float duration,int boneNum);
	~Animation();
	void DeleteAnimTree(AnimNode* node);

	void setRootNode(AnimNode* rootNode)
	{
		this->rootNode = rootNode;
	}

	void StartAnim();
	void PauseAnim();

	bool getBoneTransform(float animTime,glm::mat4 transforms);
};