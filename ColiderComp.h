#pragma once

#include"PhysicMotion.h"

//�R���C�_�[�̎��
enum class ColliderType
{
	Box
};

//�Փˎ��̖@���Ȃǂ��L�^
struct CollisionInfo
{
	//���g�̑��x
	glm::vec3 myVelocity;

	//����̑��x
	glm::vec3 partnerVelocity;

	//�@��
	glm::vec3 normal;

	//�Փ˂�������̕����I����
	PhysicParam param;

	CollisionInfo()
	{
		normal = glm::vec3(0.0f);
	}
};

struct ColiderComp
{
	size_t entityID;

	//�R���C�_�[��ID
	size_t ID;

	//�R���C�_�[�̃^�C�v
	ColliderType type;

	//�R���C�_�[�̃T�C�Y�┼�a�Ȃǂ̃p�����[�^
	//���݂̓{�b�N�X�̂�
	//���̒l�́A3D���f���̃X�P�[���ɂ���ɁA�|�����킳���
	//���ۂ�3D���f�������R���C�_�[�̂ݑ傫���������Ƃ��悤
	float size;

	bool freeze;

	//�R���C�_�[�̊e�I�t�Z�b�g
	glm::vec3 offsetPos;
	glm::vec3 offsetScale;

	CollisionInfo collision;

	ColiderComp(const size_t& entity)
	{
		entityID = entity;

		ID = 0;
		type = ColliderType::Box;
		size = 1.0f;

		offsetPos = glm::vec3(0.0f);
		offsetScale = glm::vec3(0.0f);

		collision = CollisionInfo{};
	};
};