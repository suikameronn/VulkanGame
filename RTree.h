#pragma once

#include<iostream>
#include<array>
#include<vector>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>

#define RNodeMAX 3

template<typename T>
class RNode
{
private:
	//�m�[�h�̓��̃I�u�W�F�N�g��AABB���ω������ꍇ��true�Ƃ�
	//���̃m�[�h��AABB���X�V����
	bool isUpdate;

	//�e�m�[�h
	RNode<T>* parent;

	//�q�m�[�h�̐�
	int childNodeCount;

	//�q�m�[�h
	std::array<RNode<T>*,RNodeMAX> children;

	//�m�[�h���̃I�u�W�F�N�g�̐�
	int objCount;

	//�m�[�h���̃I�u�W�F�N�g
	std::array<T*, RNodeMAX> nodeObject;

	//�m�[�h�͈̔�
	glm::vec3 min;
	glm::vec3 max;

public:

	RNode(RNode<T>* parent, glm::vec3 newMin, glm::vec3 newMax);
	RNode(RNode<T>* parent, std::vector<RNode<T>*>& srcChildren);
	RNode(RNode<T>* parent, std::vector<T*>& objects);

	~RNode();
	
	void addChildNode(RNode<T>* child)
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
	bool isOverlap(glm::vec3 newMin, glm::vec3 newMax, T* model);

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
				T* tmp = nodeObject[i];
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
				RNode<T>* tmp = children[i];
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
	void split(T* model, glm::vec3 min, glm::vec3 max);
	void split(RNode<T>* node, glm::vec3 min, glm::vec3 max);

	//�I�u�W�F�N�g��RTree�ɒǉ�����
	void insert(T* model, glm::vec3 min, glm::vec3 max);
	
	//���������ۂ̂��ꂼ��̃m�[�h�Ɏ��߂�G���g���̃C���f�b�N�X�����߂�
	std::array<std::vector<int>, 2> linearCostSplit(std::array<T*, RNodeMAX + 1>& objects,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	std::array<std::vector<int>, 2> linearCostSplit(std::array<RNode<T>*, RNodeMAX + 1>& nodes,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	//�������ő�̃y�A�̃m�[�h��Ԃ�
	std::array<int, 2> calcFarthestNodePair(std::array<T*, RNodeMAX + 1>& objects,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	std::array<int, 2> calcFarthestNodePair(std::array<RNode<T>*, RNodeMAX + 1>& nodes,
		glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex);

	//�I�u�W�F�N�g���ړ�����ۂɂ�΂�A���̃m�[�h��AABB���X�V������t���b�O�𗧂Ă�
	void requestUpdate() { isUpdate = true; }

	//�I�u�W�F�N�g����������
	void deleteObject(T* model);

	//�m�[�h���폜����
	void deleteNode(RNode<T>* childNode);

	//�^����ꂽ�͈͂̂��ׂāA���邢�͈ꕔ������m�[�h�̃I�u�W�F�N�g��z��ɓ����
	void searchInRange(std::vector<T*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max);
};

template<typename T>
class RTree
{
private:

	RNode<T>* root;

public:

	RTree()
	{
		root = new RNode<T>(nullptr, glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX));

		root->addChildNode(new RNode<T>(root, glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)));

		root->getChild(0)->updateAABB();
	}

	~RTree()
	{
		delete root;
	}

	RNode<T>* getRoot() { return root; }

	void insert(T* model, glm::vec3 min, glm::vec3 max);

	//�I�u�W�F�N�g���ړ������ꍇ�A���̃I�u�W�F�N�g����x�������A�Ăі؂ɓo�^����
	void reflectMove(T* model, RNode<T>* currentNode);

	//RTree�̃m�[�h����T���Ώۂ̃I�u�W�F�N�g�̔z���p�ӂ���
	void broadPhaseCollisionDetection(std::vector<T*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max);
};


#include"Model.h"

#include"RTree.h"

template<typename T>
RNode<T>::RNode(RNode<T>* parent, glm::vec3 newMin, glm::vec3 newMax)
{
	this->parent = parent;

	isUpdate = true;
	objCount = 0;
	childNodeCount = 0;

	min = newMin;
	max = newMax;

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

template<typename T>
RNode<T>::RNode(RNode<T>* parent, std::vector<T*>& objects)
{
	this->parent = parent;

	isUpdate = true;
	objCount = static_cast<int>(objects.size());

	childNodeCount = 0;

	std::fill(nodeObject.begin(), nodeObject.end(), nullptr);

	//�m�[�h�ɃI�u�W�F�N�g���R�s�[
	std::copy(objects.begin(), objects.end(), nodeObject.begin());

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

template<typename T>
RNode<T>::RNode(RNode<T>* parent, std::vector<RNode<T>*>& srcChildren)
{
	this->parent = parent;

	isUpdate = true;
	objCount = 0;
	childNodeCount = static_cast<int>(srcChildren.size());

	std::fill(this->children.begin(), this->children.end(), nullptr);

	//�m�[�h�ɃI�u�W�F�N�g���R�s�[
	std::copy(srcChildren.begin(), srcChildren.end(), this->children.begin());

	//���������Е��̃m�[�h�ɕ��z���ꂽ�e�m�[�h������ɍX�V
	for (int i = 0; i < childNodeCount; i++)
	{
		children[i]->parent = this;
	}

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

template<typename T>
RNode<T>::~RNode()
{
	for (int i = 0; i < childNodeCount; i++)
	{
		if (children[i] != nullptr)
		{
			delete children[i];
		}
	}
}

//���̃m�[�h��AABB���X�V
template<typename T>
void RNode<T>::updateAABB()
{
	min = glm::vec3(FLT_MAX);
	max = glm::vec3(-FLT_MAX);

	//�܂��m�[�h���̃I�u�W�F�N�g��AABB������悤��
	//�m�[�h��AABB���g��
	for (int i = 0; i < objCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			min[j] = std::min(min[j], nodeObject[i]->getMbrMin()[j]);
			max[j] = std::max(max[j], nodeObject[i]->getMbrMax()[j]);
		}
	}

	//�q�m�[�h��AABB�����ׂē����AABB�����
	for (int i = 0; i < childNodeCount; i++)
	{
		children[i]->genAABB(min, max);
	}
}

//���̃m�[�h��AABB���X�V(���ԃm�[�h)
template<typename T>
void RNode<T>::updateAABB(int index)
{
	//AABB���ύX���ꂽ�q�m�[�h��AABB���������AABB���v�Z����
	children[index]->genAABB(min, max);
}

////�I�u�W�F�N�g����m�[�h�̎Q�Ƃ��X�V
template<typename T>
void RNode<T>::updateRefNode()
{
	if (childNodeCount == 0)
	{
		for (int i = 0; i < objCount; i++)
		{
			nodeObject[i]->setRNode(this);
		}
	}
}

template<typename T>
void RNode<T>::genAABB(glm::vec3& newMin, glm::vec3& newMax)
{
	for (int i = 0; i < 3; i++)
	{
		newMin[i] = std::min(min[i], newMin[i]);
		newMax[i] = std::max(max[i], newMax[i]);
	}
}

//�ł������̗��ꂽ�I�u�W�F�N�g�̃y�A�̃C���f�b�N�X��Ԃ�(���[�t�m�[�h��)
template<typename T>
std::array<int, 2> RNode<T>::calcFarthestNodePair(std::array<T*, RNodeMAX + 1>& objects,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
	//������v�Z
	std::array<float, 3> distance;
	for (int i = 0; i < 3; i++)
	{
		distance[i] = allObjMax[i] - allObjMin[i];
	}

	float farthestDist = -FLT_MAX;
	glm::vec3 tMin, tMax;
	std::array<int, 2> pairNodeIndex;

	//���K�����ꂽ�������ł��傫���m�[�h�̃y�A��T��
	for (int i = 0; i < 3; i++)
	{
		tMin = objects[minIndex[i]]->getMbrMax();
		tMax = objects[maxIndex[i]]->getMbrMin();

		float dist = tMax[i] - tMin[i];
		dist /= distance[i];

		if (farthestDist < dist)
		{
			farthestDist = dist;
			pairNodeIndex[0] = minIndex[i];
			pairNodeIndex[1] = maxIndex[i];
		}
	}

	return pairNodeIndex;
}

//�ł������̗��ꂽ�I�u�W�F�N�g�̃y�A�̃C���f�b�N�X��Ԃ�(���ԃm�[�h��)
template<typename T>
std::array<int, 2> RNode<T>::calcFarthestNodePair(std::array<RNode<T>*, RNodeMAX + 1>& nodes,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
	//������v�Z
	std::array<float, 3> distance;
	for (int i = 0; i < 3; i++)
	{
		distance[i] = allObjMax[i] - allObjMin[i];
	}

	float farthestDist = -FLT_MAX;
	glm::vec3 tMin, tMax;
	std::array<int, 2> pairNodeIndex;

	//���K�����ꂽ�������ł��傫���m�[�h�̃y�A��T��
	for (int i = 0; i < 3; i++)
	{
		tMin = nodes[minIndex[i]]->getMbrMax();
		tMax = nodes[maxIndex[i]]->getMbrMin();

		float dist = tMax[i] - tMin[i];
		dist /= distance[i];

		if (farthestDist < dist)
		{
			farthestDist = dist;
			pairNodeIndex[0] = minIndex[i];
			pairNodeIndex[1] = maxIndex[i];
		}
	}

	return pairNodeIndex;
}

////���������ۂ̂��ꂼ��̃m�[�h�Ɏ��߂�G���g���̃C���f�b�N�X�����߂�(�q�m�[�h��)
template<typename T>
std::array<std::vector<int>, 2> RNode<T>::linearCostSplit(std::array<T*, RNodeMAX + 1>& objects,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
	//�����Е��̃m�[�h�ɓ����I�u�W�F�N�g���܂Ƃ߂�
	std::vector<int> thisNodeObjectIndex;
	std::vector<int> anotherNodeObjectIndex;

	//�ł������̗��ꂽ�I�u�W�F�N�g�̃y�A�̃C���f�b�N�X�𒲂ׂ�
	std::array<int, 2> pairNodeIndex = calcFarthestNodePair(objects, allObjMax, allObjMin, minIndex, maxIndex);

	//���ꂼ��ł����ꂽ�I�u�W�F�N�g�A�ʁX�̃m�[�h�ɓ����
	thisNodeObjectIndex.push_back(pairNodeIndex[0]);
	anotherNodeObjectIndex.push_back(pairNodeIndex[1]);

	//���ꂼ��̃m�[�h�ɃI�u�W�F�N�g��o�^����Ƃ��̃C���f�b�N�X
	//���ɗ����̃m�[�h�ɂ͍ł����ꂽ�I�u�W�F�N�g����������Ă���
	for (int i = 0; i < objects.size(); i++)
	{
		//�ő�̋��������y�A�͏���
		if (i == pairNodeIndex[0] || i == pairNodeIndex[1])
		{
			continue;
		}

		//���̂ق��̃I�u�W�F�N�g�̕��z�ɂ��Ă̓����_���ɕ��z����
		if (std::rand() % 2)
		{
			thisNodeObjectIndex.push_back(i);
		}
		else
		{
			anotherNodeObjectIndex.push_back(i);
		}
	}

	return { thisNodeObjectIndex,anotherNodeObjectIndex };
}

//���������ۂ̂��ꂼ��̃m�[�h�Ɏ��߂�G���g���̃C���f�b�N�X�����߂�(���ԃm�[�h��)
template<typename T>
std::array<std::vector<int>, 2> RNode<T>::linearCostSplit(std::array<RNode<T>*, RNodeMAX + 1>& nodes,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
	//�����Е��̃m�[�h�ɓ����I�u�W�F�N�g���܂Ƃ߂�
	std::vector<int> thisNodeObjectIndex;
	std::vector<int> anotherNodeObjectIndex;

	//�ł������̗��ꂽ�I�u�W�F�N�g�̃y�A�̃C���f�b�N�X�𒲂ׂ�
	std::array<int, 2> pairNodeIndex = calcFarthestNodePair(nodes, allObjMax, allObjMin, minIndex, maxIndex);

	//���ꂼ��ł����ꂽ�I�u�W�F�N�g�A�ʁX�̃m�[�h�ɓ����
	thisNodeObjectIndex.push_back(pairNodeIndex[0]);
	anotherNodeObjectIndex.push_back(pairNodeIndex[1]);

	//���ꂼ��̃m�[�h�ɃI�u�W�F�N�g��o�^����Ƃ��̃C���f�b�N�X
	//���ɗ����̃m�[�h�ɂ͍ł����ꂽ�I�u�W�F�N�g����������Ă���
	for (int i = 0; i < RNodeMAX + 1; i++)
	{
		//�ő�̋��������y�A�͏���
		if (i == pairNodeIndex[0] || i == pairNodeIndex[1])
		{
			continue;
		}

		//���̂ق��̃I�u�W�F�N�g�̕��z�ɂ��Ă̓����_���ɕ��z����
		if (std::rand() % 2)
		{
			thisNodeObjectIndex.push_back(i);
		}
		else
		{
			anotherNodeObjectIndex.push_back(i);
		}
	}

	return { thisNodeObjectIndex,anotherNodeObjectIndex };
}

//���݂̃m�[�h�𕪊����āA�쐬������������̃m�[�h��Ԃ�(���[�t�m�[�h��)
template<typename T>
void RNode<T>::split(T* model, glm::vec3 min, glm::vec3 max)
{
	//�e���̍ő�l�ƍŏ��l���v�Z����
	//���̒l�����m�[�h�̃C���f�b�N�X���L�^����
	glm::vec3 allObjMin = glm::vec3(FLT_MAX), allObjMax = glm::vec3(-FLT_MAX);
	glm::ivec3 minIndex, maxIndex;

	//�I�u�W�F�N�g���m�[�h�ɒǉ��������̔z���ǉ�����
	std::array<T*, RNodeMAX + 1> objects;
	for (int i = 0; i < RNodeMAX; i++)
	{
		objects[i] = nodeObject[i];
	}
	objects[RNodeMAX] = model;

	//�z��Ɋe�I�u�W�F�N�g�̊e���̍ő�l�ƍŏ��l���L�^����
	//�e���̐������A�I�u�W�F�N�g�̐������ő�l�ƍŏ��l�Ƃ��̃I�u�W�F�N�g�̃C���f�b�N�X�̃y�A������
	std::array<std::array<std::pair<float, int>, RNodeMAX + 1>, 3> maxMBR;
	std::array<std::array<std::pair<float, int>, RNodeMAX + 1>, 3> minMBR;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < RNodeMAX + 1; j++)
		{
			maxMBR[i][j].first = objects[j]->getMbrMax()[i];
			minMBR[i][j].first = objects[j]->getMbrMin()[i];

			maxMBR[i][j].second = j;
			minMBR[i][j].second = j;
		}
	}

	//�\�[�g���� �������̍ۍő�l�ƍŏ��l����Ƀ\�[�g���āA���̃I�u�W�F�N�g�̃C���f�b�N�X�͂��̓����ɕt������
	for (int i = 0; i < 3; i++)
	{
		//�~���ɂ���
		std::sort(maxMBR[i].begin(), maxMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first > b.first;
			});

		//����
		std::sort(minMBR[i].begin(), minMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first < b.first;
			});

		//��ԑ傫���l�����C���f�b�N�X���L�^
		maxIndex[i] = maxMBR[i][0].second;

		//��ԏ������l�����C���f�b�N�X���L�^
		minIndex[i] = minMBR[i][0].second;

		//�������A���ꂼ�ꓯ���C���f�b�N�X���L�^����ꍇ�͓�Ԗڂ̃C���f�b�N�X���L�^����
		if (minIndex[i] == maxIndex[i])
		{
			minIndex[i] = minMBR[i][1].second;
		}
	}

	//LinearCost�A���S���Y���ł��ꂼ��̃m�[�h�Ɏ��߂�G���g���̃C���f�b�N�X�����߂�
	std::array<std::vector<int>, 2> pairNodeObjectIndex = linearCostSplit(objects, allObjMax, allObjMin, minIndex, maxIndex);

	//��������̃m�[�h�ɓo�^����I�u�W�F�N�g���L�^���邽�߂̔z��
	std::vector<T*> anotherObjects(pairNodeObjectIndex[1].size());

	//���̃m�[�h�̃I�u�W�F�N�g������������
	std::fill(nodeObject.begin(), nodeObject.end(), nullptr);

	//�C���f�b�N�X���炻�ꂼ��̃m�[�h�ɃI�u�W�F�N�g��o�^����
	for (int i = 0; i < pairNodeObjectIndex[0].size(); i++)
	{
		nodeObject[i] = objects[pairNodeObjectIndex[0][i]];
	}
	for (int i = 0; i < pairNodeObjectIndex[1].size(); i++)
	{
		anotherObjects[i] = objects[pairNodeObjectIndex[1][i]];
	}

	//���̃m�[�h�̃I�u�W�F�N�g�̐���ݒ肷��
	objCount = static_cast<int>(pairNodeObjectIndex[0].size());

	//��������̃m�[�h���쐬���A�I�u�W�F�N�g��o�^����
	RNode<T>* anotherNode = new RNode(parent, anotherObjects);

	//���݂��̃m�[�h��AABB���X�V
	updateAABB();
	anotherNode->updateAABB();

	//�I�u�W�F�N�g�ɏ�������m�[�h��ʒm����
	updateRefNode();
	anotherNode->updateRefNode();

	//���ԃm�[�h�ɂ����Е��̃m�[�h��o�^����
	if (parent)
	{
		parent->addChildNode(anotherNode);
	}
	else
	{
		throw std::runtime_error("This Node is not Leaf Node!");
	}
}

//���݂̃m�[�h�𕪊����āA�쐬������������̃m�[�h��Ԃ�(���ԃm�[�h��)
template<typename T>
void RNode<T>::split(RNode<T>* node, glm::vec3 min, glm::vec3 max)
{
	//�e���̍ő�l�ƍŏ��l���v�Z����
	//���̒l�����m�[�h�̃C���f�b�N�X���L�^����
	glm::vec3 allNodeMin = glm::vec3(FLT_MAX), allNodeMax = glm::vec3(-FLT_MAX);
	glm::ivec3 minIndex, maxIndex;

	//�I�u�W�F�N�g���m�[�h�ɒǉ��������̔z���ǉ�����
	std::array<RNode<T>*, RNodeMAX + 1> nodes;
	for (int i = 0; i < RNodeMAX; i++)
	{
		nodes[i] = children[i];
	}
	nodes[RNodeMAX] = node;

	//�������񂱂̃m�[�h�ɓo�^���ꂽ�I�u�W�F�N�g������������
	std::fill(children.begin(), children.end(), nullptr);

	//�z��Ɋe�I�u�W�F�N�g�̊e���̍ő�l�ƍŏ��l���L�^����
	//�e���̐������A�I�u�W�F�N�g�̐������ő�l�ƍŏ��l�Ƃ��̃I�u�W�F�N�g�̃C���f�b�N�X�̃y�A������
	std::array<std::array<std::pair<float, int>, RNodeMAX + 1>, 3> maxMBR;
	std::array<std::array<std::pair<float, int>, RNodeMAX + 1>, 3> minMBR;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < RNodeMAX + 1; j++)
		{
			maxMBR[i][j].first = nodes[j]->getMbrMax()[i];
			minMBR[i][j].first = nodes[j]->getMbrMin()[i];

			maxMBR[i][j].second = j;
			minMBR[i][j].second = j;
		}
	}

	//�\�[�g���� �������̍ۍő�l�ƍŏ��l����Ƀ\�[�g���āA���̃I�u�W�F�N�g�̃C���f�b�N�X�͂��̓����ɕt������
	for (int i = 0; i < 3; i++)
	{
		//�~���ɂ���
		std::sort(maxMBR[i].begin(), maxMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first > b.first;
			});

		//����
		std::sort(minMBR[i].begin(), minMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first < b.first;
			});

		//��ԑ傫���l�����C���f�b�N�X���L�^
		maxIndex[i] = maxMBR[i][0].second;

		//��ԏ������l�����C���f�b�N�X���L�^
		minIndex[i] = minMBR[i][0].second;

		//�������A���ꂼ�ꓯ���C���f�b�N�X���L�^����ꍇ�͓�Ԗڂ̃C���f�b�N�X���L�^����
		if (minIndex[i] == maxIndex[i])
		{
			minIndex[i] = minMBR[i][1].second;
		}
	}

	//LinearCost�A���S���Y���ł��ꂼ��̃m�[�h�Ɏ��߂�G���g���̃C���f�b�N�X�����߂�
	std::array<std::vector<int>, 2> pairNodeObjectIndex = linearCostSplit(nodes, allNodeMax, allNodeMin, minIndex, maxIndex);

	//�m�[�h�ɓo�^����I�u�W�F�N�g���L�^���邽�߂̔z��
	std::vector<RNode<T>*> nodeChildren(pairNodeObjectIndex[0].size());
	std::vector<RNode<T>*> anotherNodeChildren(pairNodeObjectIndex[1].size());

	//�C���f�b�N�X���炻�ꂼ��̃m�[�h�ɃI�u�W�F�N�g��o�^����
	for (int i = 0; i < pairNodeObjectIndex[0].size(); i++)
	{
		nodeChildren[i] = nodes[pairNodeObjectIndex[0][i]];
	}
	for (int i = 0; i < pairNodeObjectIndex[1].size(); i++)
	{
		anotherNodeChildren[i] = nodes[pairNodeObjectIndex[1][i]];
	}

	//���ԃm�[�h�ɂ����Е��̃m�[�h��o�^����
	if (parent)
	{//���̃m�[�h�����ԃm�[�h�̏ꍇ

		//���̃m�[�h�̎q�m�[�h�̐����X�V����
		childNodeCount = static_cast<int>(pairNodeObjectIndex[0].size());

		std::copy(nodeChildren.begin(), nodeChildren.end(), children.begin());

		//�q�m�[�h�̐e��ݒ肵�Ȃ���
		for (int i = 0; i < childNodeCount; i++)
		{
			children[i]->parent = this;
		}

		//��������̃m�[�h���쐬���A�q�m�[�h��o�^����
		//�q�m�[�h�̐e�̓R���X�g���N�^��Őݒ肷��
		RNode<T>* anotherNode = new RNode(parent, anotherNodeChildren);

		//���݂��̃m�[�h��AABB���X�V
		updateAABB();
		anotherNode->updateAABB();

		parent->addChildNode(anotherNode);
	}
	else
	{//���̃m�[�h�����[�g�m�[�h�̏ꍇ
	 //���̃m�[�h�𕪊�����Ƃ������A�V���ɓ�̃m�[�h�����A
	 //���̕Е��ɂ��̃m�[�h�̃I�u�W�F�N�g�Ȃǂ������p���A
	 //���̃m�[�h�̓f�[�^�����Z�b�g���āA���[�g�m�[�h�Ƃ��Ďg��

		RNode<T>* node1 = new RNode(this, nodeChildren);

		//��������̃m�[�h���쐬���A�I�u�W�F�N�g��o�^����
		RNode<T>* node2 = new RNode(this, anotherNodeChildren);

		//���݂��̃m�[�h��AABB���X�V
		node1->updateAABB();
		node2->updateAABB();

		//���̃m�[�h�̃f�[�^�����Z�b�g���āA�Ăу��[�g�m�[�h�ɂ���
		resetToRootNode();

		//���[�g�m�[�h�ɕ��������m�[�h��o�^����
		this->addChildNode(node1);
		this->addChildNode(node2);

		//���[�g�m�[�h��AABB���X�V
		this->updateAABB();
	}
}

template<typename T>
void RNode<T>::insert(T* model, glm::vec3 min, glm::vec3 max)
{
	if (childNodeCount == 0)
	{//���[�t�m�[�h�̏ꍇ

		if (objCount >= RNodeMAX)
		{//�m�[�h�̃I�u�W�F�N�g�����t�Ȏ��͕������s��

			//�������s������A���ԃm�[�h�ɕ�����̃m�[�h��n��
			split(model, min, max);
		}
		else
		{//�󂫂�����Ƃ��́A�m�[�h�ɃI�u�W�F�N�g��ǉ����AAABB���g������

			nodeObject[objCount] = model;
			model->setRNode(this);

			objCount++;

			//�I�u�W�F�N�g��AABB���܂ނ悤�Ƀm�[�h��AABB���g������
			expandAABB(min, max);
		}
	}
	else
	{//���[�t�m�[�h�ł͂Ȃ��ꍇ

		int closestNodeIndex = -1;
		float newAABBVolume = FLT_MAX;
		glm::vec3 newMin, newMax;

		for (int i = 0; i < static_cast<int>(childNodeCount); i++)
		{
			newMin = min;
			newMax = max;

			//�I�u�W�F�N�g��ǉ������Ƃ���
			//������܂�AABB���v�Z����
			children[i]->genAABB(newMin, newMax);

			//�I�u�W�F�N�g��ǉ������Ƃ���AABB�̑̐ς̑����ʂ��v�Z����
			//(�̐ς̑�����) = (�ǉ�������̑̐�) - (���̑̐�)
			float volume = calcVolume(newMin, newMax) - children[i]->calcVolume();

			//�̐ς̑����ʂ��ŏ��ƂȂ�m�[�h�����߂�
			if (volume > 0.0f && newAABBVolume > volume)
			{
				newAABBVolume = volume;
				closestNodeIndex = i;
			}
		}

		//�����A���ׂĂ̎q�m�[�h���I�u�W�F�N�g��AABB�����S�ɓ����ꍇ
		//�ł����̑̐ς̏������m�[�h��I������
		if (closestNodeIndex == -1)
		{
			for (int i = 0; i < static_cast<int>(childNodeCount); i++)
			{
				float volume = children[i]->calcVolume();

				if (newAABBVolume > volume)
				{
					newAABBVolume = volume;
					closestNodeIndex = i;
				}
			}
		}

		//��ԍŏ��̃��[�g�m�[�h�̎q�m�[�h�ɃI�u�W�F�N�g������ہA��ڂ̃m�[�h�ɃI�u�W�F�N�g������
		if (closestNodeIndex == -1)
		{
			closestNodeIndex = 0;
		}

		//���[�t�m�[�h�ڎw���āA�m�[�h������
		children[closestNodeIndex]->insert(model, min, max);

		//�I�u�W�F�N�g�����[�t�m�[�h�ɒǉ�������
		//�q�m�[�h�̊g�����ꂽAABB�����ׂē����
		//AABB���v�Z����

		updateAABB();
	}
}

//�f�[�^�����Z�b�g���āA���̃m�[�h�����[�g�m�[�h�ɖ߂�
template<typename T>
void RNode<T>::resetToRootNode()
{
	//�m�[�h��I�u�W�F�N�g���̏�����
	childNodeCount = 0;
	objCount = 0;

	//�o�^���ꂽ�f�[�^�̏�����
	std::fill(nodeObject.begin(), nodeObject.end(), nullptr);
	std::fill(children.begin(), children.end(), nullptr);

	//���[�g�m�[�h��AABB��������
	min = glm::vec3(-FLT_MAX);
	max = glm::vec3(FLT_MAX);
}

//�m�[�h���̃I�u�W�F�N�g���폜����
template<typename T>
void RNode<T>::deleteObject(T* model)
{
	//�Y���̃I�u�W�F�N�g���폜���A�I�u�W�F�N�g�̃|�C���^��擪�ɋl�߂�
	for (int i = 0; i < objCount; i++)
	{
		//�Y���I�u�W�F�N�g���m�[�h����폜����
		if (nodeObject[i] == model)
		{
			nodeObject[i] = nullptr;
		}
	}

	//�I�u�W�F�N�g���̃m�[�h�̓o�^���폜����
	model->setRNode(nullptr);

	sortNodeObject();

	//�I�u�W�F�N�g�������炷
	objCount--;

	//�����܂łŃm�[�h���̃I�u�W�F�N�g�̔z��͐��ڂ���
	//�z��͐擪�Ɍ����ăI�u�W�F�N�g�ւ̎Q�Ƃ��l�߂��A�Ԃ�nullptr�����܂��Ă��Ȃ���Ԃ�

	//���̎��A�I�u�W�F�N�g����0�ɂȂ�����A���̃m�[�h���̂��폜���A��̃m�[�h��AABB����������
	if (objCount <= 0)
	{
		//�e�m�[�h���玩�����폜����
		parent->deleteNode(this);

		delete this;
	}
}

//�q�m�[�h�̓o�^���폜����
template<typename T>
void RNode<T>::deleteNode(RNode<T>* childNode)
{
	for (int i = 0; i < childNodeCount; i++)
	{
		if (children[i] == childNode)
		{
			children[i] = nullptr;

			break;
		}
	}

	//nullptr���O�̂ق��ɗ��Ȃ��悤�ɐ���
	sortChildNode();

	childNodeCount--;

	if (childNodeCount == 0)
	{
		parent->deleteNode(this);

		delete this;
	}
}

//�ꕔ�ł��d�Ȃ��Ă�����true
template<typename T>
bool RNode<T>::isOverlap(glm::vec3 newMin, glm::vec3 newMax)
{
	bool isOverlap = true;

	for (int i = 0; i < 3; i++)
	{
		if (min[i] > newMax[i] || max[i] < newMin[i])
		{
			isOverlap = false;
		}
	}

	return isOverlap;
}

//�ꕔ�ł��d�Ȃ��Ă�����true
template<typename T>
bool RNode<T>::isOverlap(glm::vec3 newMin, glm::vec3 newMax, T* model)
{
	bool isOverlap = true;

	for (int i = 0; i < 3; i++)
	{
		if (model->getMbrMin()[i] > newMax[i] || model->getMbrMax()[i] < newMin[i])
		{
			isOverlap = false;
		}
	}

	return isOverlap;
}

//�^����ꂽ�͈͂̂��ׂāA���邢�͈ꕔ������m�[�h�̃I�u�W�F�N�g��z��ɓ����
template<typename T>
void RNode<T>::searchInRange(std::vector<T*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max)
{
	//�܂����̃m�[�h���^����ꂽ�͈͂̈ꕔ�����ׂĂ����邩�ǂ����𒲂ׂ�
	if (isOverlap(min, max))
	{
		//���q�m�[�h�։�����A�����ōĂє͈͒T�����s��
		for (int i = 0; i < childNodeCount; i++)
		{
			children[i]->searchInRange(collisionDetectTarget, min, max);
		}

		//�^����ꂽ�͈͂ƃm�[�h��MBR���ꕔ�ł��d�Ȃ��Ă�����
		for (int i = 0; i < objCount; i++)
		{
			//�I�u�W�F�N�g��MBR�Ƃ��d�Ȃ��Ă�����
			//�̂��ɏڍׂȓ����蔻����s�����߂ɔz��ɃI�u�W�F�N�g��������
			if (isOverlap(min, max, nodeObject[i]))
			{
				collisionDetectTarget.push_back(nodeObject[i]);
			}
		}
	}
}

template<typename T>
void RTree<T>::insert(T* model, glm::vec3 min, glm::vec3 max)
{
	root->insert(model, min, max);
}

//�I�u�W�F�N�g�ړ������Ƃ��̊֐�
//�I�u�W�F�N�g����؂���폜���A�Ăђǉ�����
template<typename T>
void RTree<T>::reflectMove(T* model, RNode<T>* currentNode)
{
	//�܂��I�u�W�F�N�g�̏�������m�[�h���玩�����폜����
	currentNode->deleteObject(model);

	//�ĂѓK�؂ȃm�[�h�ɑ}������
	//�I�u�W�F�N�g�ɂ�insert�֐����ŐV���ɏ�������m�[�h��ݒ肳���
	insert(model, model->getMbrMin(), model->getMbrMax());
}

//�^����ꂽMBR����ڍׂȓ����蔻����s���K�v������I�u�W�F�N�g��z��ɂ܂Ƃ߂�
template<typename T>
void RTree<T>::broadPhaseCollisionDetection(std::vector<T*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max)
{
	root->searchInRange(collisionDetectTarget, min, max);
}