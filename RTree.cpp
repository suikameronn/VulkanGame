#include"Model.h"

#include"RTree.h"

//���̃m�[�h��AABB���X�V
void RNode::updateAABB()
{
	min = glm::vec3(FLT_MAX);
	max = glm::vec3(-FLT_MAX);

	//�܂��m�[�h���̃I�u�W�F�N�g��AABB������悤��
	//�m�[�h��AABB���g��
	for (int i = 0; i < objCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			min[j] = std::min(min[j], nodeObject[i].lock()->getMin()[j]);
			max[j] = std::max(max[j], nodeObject[i].lock()->getMax()[j]);
		}
	}

	//�q�m�[�h��AABB�����ׂē����AABB�����
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->genAABB(min, max);
	}
}

//���̃m�[�h��AABB���X�V(���ԃm�[�h)
void RNode::updateAABB(int index)
{
	//AABB���ύX���ꂽ�q�m�[�h��AABB���������AABB���v�Z����
	children[index]->genAABB(min, max);
}

void RNode::genAABB(glm::vec3& newMin, glm::vec3& newMax)
{
	for (int i = 0; i < 3; i++)
	{
		newMin[i] = std::min(min[i], newMin[i]);
		newMax[i] = std::max(max[i], newMax[i]);
	}
}

//���݂̃m�[�h�𕪊����āA�쐬������������̃m�[�h��Ԃ�
RNode* RNode::split(std::weak_ptr<Model> model, glm::vec3 min, glm::vec3 max)
{
	//�e���̍ő�l�ƍŏ��l���v�Z����
	//���̒l�����m�[�h�̃C���f�b�N�X���L�^����
	glm::vec3 allObjMin, allObjMax;
	glm::ivec3 minIndex, maxIndex;

	//�I�u�W�F�N�g���m�[�h�ɒǉ��������̔z���ǉ�����
	std::array<std::weak_ptr<Model>, RNodeMAX + 1> objects;
	for (int i = 0; i < RNodeMAX; i++)
	{
		objects[i] = nodeObject[i];
	}
	objects[RNodeMAX + 1] = model;


	//��̔z��̃I�u�W�F�N�g��AABB�̊e���̍ŏ��l�ƍő�l���擾
	for (int i = 0; i < RNodeMAX + 1; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			//�m�[�h����AABB����e���̍ŏ��l�A�ő�l���擾
			glm::vec3 objMin, objMax;
			objects[i].lock()->getMinMax(objMin, objMax);

			if (allObjMin[j] > objMin[j])
			{
				allObjMin[j] = objMin[j];
				minIndex[j] = i;
			}

			if (allObjMax[j] > objMax[j])
			{
				allObjMax[j] = objMax[j];
				maxIndex[j] = i;
			}
		}
	}

	//LinearCost�A���S���Y��

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
		tMin = objects[minIndex[i]].lock()->getMax();
		tMax = objects[maxIndex[i]].lock()->getMin();

		float dist = tMax[i] - tMin[i];
		dist /= distance[i];

		if (farthestDist < dist)
		{
			farthestDist = dist;
			pairNodeIndex[0] = minIndex[i];
			pairNodeIndex[1] = maxIndex[i];
		}
	}

	//�����̃m�[�h��AABB���X�V ���̏ꍇ�͏k��


	//RNode* node = new RNode();
}

void RNode::insert(std::weak_ptr<Model> model, glm::vec3 min, glm::vec3 max)
{
	if (children.size() == 0)
	{//���[�t�m�[�h�̏ꍇ

		if (objCount > RNodeMAX)
		{//�m�[�h�̃I�u�W�F�N�g�����t�Ȏ��͕������s��

			//�������s������A���ԃm�[�h�ɕ�����̃m�[�h��n��
			parent->addChildNode(split(model, min, max));
		}
		else
		{//�󂫂�����Ƃ��́A�m�[�h�ɃI�u�W�F�N�g��ǉ����AAABB���g������
			nodeObject[objCount] = model;
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

		for (int i = 0; i < children.size(); i++)
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
			for (int i = 0; i < 3; i++)
			{
				float volume = children[i]->calcVolume();

				if (newAABBVolume > volume)
				{
					newAABBVolume = volume;
					closestNodeIndex = i;
				}
			}
		}

		//���[�t�m�[�h�ڎw���āA�m�[�h������
		children[closestNodeIndex]->insert(model, min, max);

		//�I�u�W�F�N�g�����[�t�m�[�h�ɒǉ�������
		//�q�m�[�h�̊g�����ꂽAABB�����ׂē����
		//AABB���v�Z����
		updateAABB(closestNodeIndex);
	}
}

RTree* RTree::instance = nullptr;

RTree::RTree()
{
	root = new RNode(nullptr, glm::vec3(0.0f), glm::vec3(0.0f));
}

void RTree::insert(std::weak_ptr<Model> model,glm::vec3 min,glm::vec3 max)
{

}