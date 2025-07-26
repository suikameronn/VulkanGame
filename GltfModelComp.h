#pragma once

#include<iostream>

struct GltfModelComp
{
	//3D���f���ւ�ID
	uint32_t modelID;

	std::string filePath;

	GltfModelComp()
	{
		modelID = 0;

		filePath = "";
	}
};

struct GltfModelAnimComp
{
	//�A�j���[�V�����̖��O
	std::string animationName;
	//�A�j���[�V�����̍Đ����x
	float speed;
	//�A�j���[�V�����̃��[�v�t���O
	bool loop;
	//�A�j���[�V�����̍Đ��J�n����
	float startTime;

	GltfModelAnimComp()
	{
		animationName = "";
		speed = 1.0f;
		loop = true;
		startTime = 0.0f;
	}
};;