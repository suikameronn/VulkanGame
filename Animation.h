#pragma once

#include<vector>
#include<map>
#include<memory>
#include <assimp/scene.h>           // Output data structure
#include<glm/glm.hpp>

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

	std::shared_ptr<AnimNode> parent;
	std::vector<std::shared_ptr<AnimNode>> children;

	std::string name;
	AnimationKeyData animKeyData;
	glm::mat4 matrix;

public:

	AnimNode(std::shared_ptr<AnimNode> parentNode,std::string nodeName,AnimationKeyData data, unsigned int childrenCount)
	{
		animNode = true;
		parent = parentNode;
		name = nodeName;
		animKeyData = data;
		children.resize(childrenCount);
	}

	AnimNode(std::shared_ptr<AnimNode> parentNode, std::string nodeName, glm::mat4 mat,unsigned int childrenCount)
	{
		animNode = false;
		parent = parentNode;
		name = nodeName;
		matrix = mat;
	}

	void resizeChildren(int childNum)
	{
		children.resize(childNum);
	}

	void setChild(int i,std::shared_ptr<AnimNode> child)
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

	std::shared_ptr<AnimNode> getChildren(int i)
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

	std::shared_ptr<AnimNode> rootNode;

public:

	Animation(float timeInTick,float duration,int boneNum);

	void setRootNode(std::shared_ptr<AnimNode> rootNode)
	{
		this->rootNode = rootNode;
	}

	void StartAnim();
	void PauseAnim();

	void setBoneTransform(std::vector < std::vector<glm::mat4>> transforms);
	std::vector<glm::mat4> &getBoneTransform();
};