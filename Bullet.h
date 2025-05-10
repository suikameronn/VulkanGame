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
	//弾の進行方向
	glm::vec3 direction;
	//レイキャスト時のレイの長さ
	float rayLength;

	//弾の進行方向にモデルの向きを合わせる
	void calculateBulletAngle();

public:

	Bullet(float s,float length,glm::vec3 dir,glm::vec3 pos,float limit);
	~Bullet();

	void Update() override;

};