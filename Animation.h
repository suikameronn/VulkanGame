#pragma once

#include<vector>
#include<unordered_map>
#include<memory>
#include <assimp/scene.h>           // Output data structure
#include<glm/glm.hpp>

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

	glm::mat4 getAnimMatrix(float animTime)
	{
		//補間などを計算して、平行移動と回転、スケーリングの行列を合成した行列を返す
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

	void setBoneTransform(std::vector < std::vector<glm::mat4>> transforms);
	std::vector<glm::mat4> &getBoneTransform();
};