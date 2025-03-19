#pragma once
#include<time.h>
#include<vector>
#include<memory>
#include<chrono>

#include<iostream>

#include<glm/glm.hpp>

class PhysicBase
{
private:

	//重力
	glm::vec3 gravity;
	//速度
	glm::vec3 velocity;

public:

	PhysicBase();

	//速度の更新処理
	virtual void Update(bool isAccelerateReset = true);
	//速度のリセット
	void setZeroVelocity();
	//重力を打ち消す(地面に足をつけているとき)
	void cancelGravity();
	//速度を向きも含めて加える
	virtual void addVelocity(glm::vec3 addVelocity);
	//現在の速度を取得する
	virtual glm::vec3 getVelocity();
};