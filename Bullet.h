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
	//�e�̐i�s����
	glm::vec3 direction;
	//���C�L���X�g���̃��C�̒���
	float rayLength;

	//�e�̐i�s�����Ƀ��f���̌��������킹��
	void calculateBulletAngle();

public:

	Bullet(float s,float length,glm::vec3 dir,glm::vec3 pos,float limit);
	~Bullet();

	void Update() override;

};