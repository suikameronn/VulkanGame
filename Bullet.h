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

	//���g��weak_ptr��Ԃ�
	std::weak_ptr<Bullet> getThisWeakPtr()
	{
		return std::dynamic_pointer_cast<Bullet>(shared_from_this());
	}

	void Update() override;

	void collision(std::weak_ptr<Model> collideObj);
	void collision(std::weak_ptr<Player> player);
};