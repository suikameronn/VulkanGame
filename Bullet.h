#pragma once

#include"Model.h"

class Bullet : public Model
{
private:

	//�˒�����
	float distanceLimit;
	//���ˎ��̍��W
	glm::vec3 shootPos;

	//�e�̃X�s�[�h
	float speed;

	Ray ray;

	//�e�̐i�s�����Ƀ��f���̌��������킹��
	void calculateBulletAngle();

public:

	Bullet(float s,float length,glm::vec3 dir,glm::vec3 pos,float limit);
	~Bullet();

	void initFrameSetting() override;
	void Update() override;

	void collision(std::shared_ptr<Model> model) override;
};