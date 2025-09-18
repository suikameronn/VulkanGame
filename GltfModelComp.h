#pragma once

#include<iostream>

struct GltfModelComp
{
	size_t entityID;

	//3D���f���ւ�ID
	size_t modelID;

	std::string filePath;

	GltfModelComp(const size_t& entity)
	{
		entityID = entity;

		modelID = 0;

		filePath = "";
	}
};

struct GltfModelAnimComp
{
	size_t entityID;

	//�A�j���[�V�����̖��O
	std::string animationName;
	//�A�j���[�V�����̍Đ����x
	float speed;
	//�A�j���[�V�����̃��[�v�t���O
	bool loop;
	//�A�j���[�V�����̍Đ��J�n����
	float startTime;

	//�m�[�h���Ƃ̃A�j���[�V�����s��
	NodeTransform nodeTransform;

	GltfModelAnimComp(const size_t& entity)
	{
		entityID = entity;

		animationName = "";
		speed = 1.0f;
		loop = true;
		startTime = 0.0f;

		nodeTransform.init();
	}
};;