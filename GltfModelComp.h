#pragma once

#include<iostream>

struct GltfModelComp
{
	size_t entityID;

	//3DモデルへのID
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

	//アニメーションの名前
	std::string animationName;
	//アニメーションの再生速度
	float speed;
	//アニメーションのループフラグ
	bool loop;
	//アニメーションの再生開始時間
	float startTime;

	//ノードごとのアニメーション行列
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