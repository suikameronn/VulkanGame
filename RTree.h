#pragma once

#include<iostream>
#include<array>
#include<vector>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>

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

	//子ノードの数
	int childNodeCount;

	//子ノード
	std::array<RNode*,RNodeMAX> children;

	//ノード内のオブジェクトの数
	int objCount;

	//ノード内のオブジェクト
	std::array<Model*, RNodeMAX> nodeObject;

	//ノードの範囲
	glm::vec3 min;
	glm::vec3 max;

public:

	RNode(RNode* parent, glm::vec3 newMin, glm::vec3 newMax);
	RNode(RNode* parent, std::vector<RNode*>& srcChildren);
	RNode(RNode* parent, std::vector<Model*>& objects);

	~RNode();
	
	void addChildNode(RNode* child)
	{
		if (childNodeCount == RNodeMAX)
		{
			//このノードを分割して、できたもう一つのノードを親の子に加える
			split(child, child->getMbrMin(), child->getMbrMax());
		}
		else
		{
			children[childNodeCount] = child;
			childNodeCount++;

			int a = 0;
			for (int i = 0; i < 3; i++)
			{
				if (children[i] != nullptr)
				{
					a++;
				}
			}

			if (a != childNodeCount)
			{
				std::cout << a << std::endl;
			}
		}
	}

	void setBoxRange(glm::vec3 min, glm::vec3 max)
	{
		this->min = min;
		this->max = max;
	}

	glm::vec3 getMbrMin()
	{
		return min;
	}

	glm::vec3 getMbrMax()
	{
		return max;
	}

	RNode* getChild(int i)
	{
		return children[i];
	}

	int getChildrenSize()
	{
		return childNodeCount;
	}

	float calcVolume()
	{
		float volume = (max.x - min.x) * (max.y - min.y) * (max.z - min.z);

		if (volume < 0.0f)
		{
			volume = 0.0f;
		}

		return volume;
	}

	float calcVolume(glm::vec3 min, glm::vec3 max)
	{
		float volume = (max.x - min.x) * (max.y - min.y) * (max.z - min.z);

		if (volume < 0.0f)
		{
			volume = 0.0f;
		}

		return volume;
	}

	//与えられた範囲がノードの持つ範囲に一部でも重なっていたらtrueを返す
	bool isOverlap(glm::vec3 newMin, glm::vec3 newMax);
	bool isOverlap(glm::vec3 newMin, glm::vec3 newMax, Model* model);

	void expandAABB(glm::vec3 newMin, glm::vec3 newMax)
	{
		for (int i = 0; i < 3; i++)
		{
			min[i] = std::min(min[i], newMin[i]);
			max[i] = std::max(max[i], newMax[i]);
		}
	}

	//オブジェクトの配列をオブジェクトのポインタが先頭に詰められるように整頓する
	void sortNodeObject()
	{
		for (int i = 0; i < RNodeMAX - 1; i++)
		{
			if (nodeObject[i] == nullptr)
			{
				Model* tmp = nodeObject[i];
				nodeObject[i] = nodeObject[i + 1];
				nodeObject[i + 1] = tmp;
			}
		}
	}

	//オブジェクトの配列をオブジェクトのポインタが先頭に詰められるように整頓する
	void sortChildNode()
	{
		for (int i = 0; i < RNodeMAX - 1; i++)
		{
			if (children[i] == nullptr)
			{
				RNode* tmp = children[i];
				children[i] = children[i + 1];
				children[i + 1] = tmp;
			}
		}
	}

	//ノードのデータをリセットしてルートノードにする
	void resetToRootNode();

	//このノードのAABBを更新
	void updateAABB();

	//このノードのAABBを更新(中間ノード)
	void updateAABB(int index);

	//オブジェクトからノードの参照を更新
	void updateRefNode();

	void genAABB(glm::vec3& newMin, glm::vec3& newMax);

	//現在のノードを分割して、作成したもう一方のノードを返す
	void split(Model* model, glm::vec3 min, glm::vec3 max);
	void split(RNode* node, glm::vec3 min, glm::vec3 max);

	//オブジェクトをRTreeに追加する
	void insert(Model* model, glm::vec3 min, glm::vec3 max);
	
	//分割した際のそれぞれのノードに収めるエントリのインデックスを決める
	std::array<std::vector<int>, 2> linearCostSplit(std::array<Model*, RNodeMAX + 1>& objects,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	std::array<std::vector<int>, 2> linearCostSplit(std::array<RNode*, RNodeMAX + 1>& nodes,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	//距離が最大のペアのノードを返す
	std::array<int, 2> calcFarthestNodePair(std::array<Model*, RNodeMAX + 1>& objects,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	std::array<int, 2> calcFarthestNodePair(std::array<RNode*, RNodeMAX + 1>& nodes,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	//オブジェクトを移動する際によばれ、このノードのAABBを更新させるフラッグを立てる
	void requestUpdate() { isUpdate = true; }

	//オブジェクトを消去する
	void deleteObject(Model* model);

	//ノードを削除する
	void deleteNode(RNode* childNode);

	//与えられた範囲のすべて、あるいは一部を内包するノードのオブジェクトを配列に入れる
	void searchInRange(std::vector<Model*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max);
};

class RTree
{
private:
	static RTree* instance;

	RNode* root;

public:

	RTree();
	~RTree();

	RNode* getRoot() { return root; }

	void insert(Model* model, glm::vec3 min, glm::vec3 max);

	//オブジェクトが移動した場合、そのオブジェクトを一度消去し、再び木に登録する
	void reflectMove(Model* model, RNode* currentNode);

	//RTreeのノードから探索対象のオブジェクトの配列を用意する
	void broadPhaseCollisionDetection(std::vector<Model*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max);
};