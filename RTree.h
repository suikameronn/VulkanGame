#pragma once

#include<iostream>
#include<array>
#include<vector>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model;

#define RNodeMAX 3

class RNode
{
private:
	//ノードの内のオブジェクトのAABBが変化した場合はtrueとし
	//このノードのAABBを更新する
	bool isUpdate;

	//親ノード
	RNode* parent;

	//子ノード
	std::vector<RNode*> children;

	//ノード内のオブジェクトの数
	uint32_t objCount;

	//ノード内のオブジェクト
	std::array<std::weak_ptr<Model>, RNodeMAX> nodeObject;

	//ノードの範囲
	glm::vec3 min;
	glm::vec3 max;

public:

	RNode(RNode* parent,glm::vec3 newMin,glm::vec3 newMax)
	{
		this->parent = parent;

		isUpdate = true;
		objCount = 0;

		min = newMin;
		max = newMax;
	}
	
	void addChildNode(RNode* child)
	{
		children.push_back(child);
	}

	void setBoxRange(glm::vec3 min, glm::vec3 max)
	{
		this->min = min;
		this->max = max;
	}

	glm::vec3 getMin()
	{
		return min;
	}

	glm::vec3 getMax()
	{
		return max;
	}

	RNode* getChild(uint32_t i)
	{
		return children[i];
	}

	uint32_t getChildrenSize()
	{
		return static_cast<uint32_t>(children.size());
	}

	float calcVolume()
	{
		return (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
	}

	float calcVolume(glm::vec3 min, glm::vec3 max)
	{
		return (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
	}

	bool isContainAABB(glm::vec3 newMin, glm::vec3 newMax)
	{
		bool isContain = true;

		for (int i = 0; i < 3; i++)
		{
			if (min[i] > newMin[i] || max[i] < newMax[i])
			{
				isContain = false;
			}
		}

		return isContain;
	}

	void expandAABB(glm::vec3 newMin, glm::vec3 newMax)
	{
		for (int i = 0; i < 3; i++)
		{
			min[i] = std::min(min[i], newMin[i]);
			max[i] = std::max(max[i], newMax[i]);
		}
	}

	//このノードのAABBを更新
	void updateAABB();

	//このノードのAABBを更新(中間ノード)
	void updateAABB(int index);

	void genAABB(glm::vec3& newMin, glm::vec3& newMax);

	//現在のノードを分割して、作成したもう一方のノードを返す
	RNode* split(std::weak_ptr<Model> model, glm::vec3 min, glm::vec3 max);

	//オブジェクトをRTreeに追加する
	void insert(std::weak_ptr<Model> model, glm::vec3 min, glm::vec3 max);
};

class RTree
{
private:
	static RTree* instance;

	RNode* root;

public:

	static RTree* GetInstance()
	{
		if (!instance)
		{
			instance = new RTree();
		}

		return instance;
	}

	RTree();

	RNode* getRoot() { return root; }

	void insert(std::weak_ptr<Model> model, glm::vec3 min, glm::vec3 max);
};