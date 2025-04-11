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
	//�m�[�h�̓��̃I�u�W�F�N�g��AABB���ω������ꍇ��true�Ƃ�
	//���̃m�[�h��AABB���X�V����
	bool isUpdate;

	//�e�m�[�h
	RNode* parent;

	//�q�m�[�h�̐�
	int childNodeCount;

	//�q�m�[�h
	std::array<RNode*,RNodeMAX> children;

	//�m�[�h���̃I�u�W�F�N�g�̐�
	int objCount;

	//�m�[�h���̃I�u�W�F�N�g
	std::array<Model*, RNodeMAX> nodeObject;

	//�m�[�h�͈̔�
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
			//���̃m�[�h�𕪊����āA�ł���������̃m�[�h��e�̎q�ɉ�����
			split(child, child->getMbrMin(), child->getMbrMax());
		}
		else
		{
			children[childNodeCount] = child;
			childNodeCount++;
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

	//�^����ꂽ�͈͂��m�[�h�̎��͈͂Ɉꕔ�ł��d�Ȃ��Ă�����true��Ԃ�
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

	//�I�u�W�F�N�g�̔z����I�u�W�F�N�g�̃|�C���^���擪�ɋl�߂���悤�ɐ��ڂ���
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

	//�I�u�W�F�N�g�̔z����I�u�W�F�N�g�̃|�C���^���擪�ɋl�߂���悤�ɐ��ڂ���
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

	//�m�[�h�̃f�[�^�����Z�b�g���ă��[�g�m�[�h�ɂ���
	void resetToRootNode();

	//���̃m�[�h��AABB���X�V
	void updateAABB();

	//���̃m�[�h��AABB���X�V(���ԃm�[�h)
	void updateAABB(int index);

	//�I�u�W�F�N�g����m�[�h�̎Q�Ƃ��X�V
	void updateRefNode();

	void genAABB(glm::vec3& newMin, glm::vec3& newMax);

	//���݂̃m�[�h�𕪊����āA�쐬������������̃m�[�h��Ԃ�
	void split(Model* model, glm::vec3 min, glm::vec3 max);
	void split(RNode* node, glm::vec3 min, glm::vec3 max);

	//�I�u�W�F�N�g��RTree�ɒǉ�����
	void insert(Model* model, glm::vec3 min, glm::vec3 max);
	
	//���������ۂ̂��ꂼ��̃m�[�h�Ɏ��߂�G���g���̃C���f�b�N�X�����߂�
	std::array<std::vector<int>, 2> linearCostSplit(std::array<Model*, RNodeMAX + 1>& objects,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	std::array<std::vector<int>, 2> linearCostSplit(std::array<RNode*, RNodeMAX + 1>& nodes,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	//�������ő�̃y�A�̃m�[�h��Ԃ�
	std::array<int, 2> calcFarthestNodePair(std::array<Model*, RNodeMAX + 1>& objects,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	std::array<int, 2> calcFarthestNodePair(std::array<RNode*, RNodeMAX + 1>& nodes,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	//�I�u�W�F�N�g���ړ�����ۂɂ�΂�A���̃m�[�h��AABB���X�V������t���b�O�𗧂Ă�
	void requestUpdate() { isUpdate = true; }

	//�I�u�W�F�N�g����������
	void deleteObject(Model* model);

	//�m�[�h���폜����
	void deleteNode(RNode* childNode);

	//�^����ꂽ�͈͂̂��ׂāA���邢�͈ꕔ������m�[�h�̃I�u�W�F�N�g��z��ɓ����
	void searchInRange(std::vector<Model*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max);

	void debug()
	{
		//std::cout << "\nChildNodeCount " << childNodeCount << std::endl;
		
		int a = 0;
		for (int i = 0; i < children.size(); i++)
		{
			if (children[i] != nullptr)
			{
				a++;
			}
		}

		//std::cout << "Actually ChildNodeCount " << a << std::endl;

		if (a != childNodeCount)
		{
			std::cout << "Error " << std::endl;
		}

		//std::cout << "ObjCount " << objCount << std::endl;

		a = 0;
		for (int i = 0; i < nodeObject.size(); i++)
		{
			if (nodeObject[i] != nullptr)
			{
				a++;
			}
		}

		//std::cout << "Actually ObjCount " << a << std::endl;

		if (a != objCount)
		{
			std::cout << "Error" << std::endl;
		}

		for (int i = 0; i < children.size(); i++)
		{
			if (children[i] != nullptr)
			{
				children[i]->debug();
			}
		}
	}
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

	//�I�u�W�F�N�g���ړ������ꍇ�A���̃I�u�W�F�N�g����x�������A�Ăі؂ɓo�^����
	void reflectMove(Model* model, RNode* currentNode);

	//RTree�̃m�[�h����T���Ώۂ̃I�u�W�F�N�g�̔z���p�ӂ���
	void broadPhaseCollisionDetection(std::vector<Model*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max);
};