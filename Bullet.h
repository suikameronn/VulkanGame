#pragma once

#include"Model.h"

class Bullet : public Model
{
private:

	//射程距離
	float distanceLimit;
	//発射時の座標
	glm::vec3 shootPos;

	//弾のスピード
	float speed;

	Ray ray;

	//弾の進行方向にモデルの向きを合わせる
	void calculateBulletAngle();

public:

	Bullet(float s,float length,glm::vec3 dir,glm::vec3 pos,float limit);
	~Bullet();

	void initFrameSetting() override;
	void Update() override;

	void updateTransformMatrix() override;//座標変換行列の更新

	void collision(std::shared_ptr<Model> model) override;
};