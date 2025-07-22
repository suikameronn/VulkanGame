#pragma once

#include<iostream>

struct GltfModelComp
{
	//3DモデルへのID
	uint32_t modelID;

	GltfModelComp()
	{
		modelID = 0;
	}
};

struct GltfModelAnimComp
{
	//アニメーションの名前
	std::string animationName;
	//アニメーションの再生速度
	float speed;
	//アニメーションのループフラグ
	bool loop;
	//アニメーションの再生開始時間
	float startTime;

	GltfModelAnimComp()
	{
		animationName = "";
		speed = 1.0f;
		loop = true;
		startTime = 0.0f;
	}
};;