#pragma once

#include<vector>

//���C�͂Ȃǂ̕����I����
struct PhysicParam
{
	//����
	float m;

	//�Ö��C�͌W��
	float staticFriction;
	//�����C�͌W��
	float kineticFriction;

	//������
	float restitution;

	//�����̂̃R���C�_�[�̒f�ʐ�
	std::array<std::pair<glm::vec3, float>, 8> crossArea;

	PhysicParam()
	{
		m = 1.0f;

		staticFriction = 0.0f;
		kineticFriction = 0.0f;

		restitution = 0.0f;
	}

	//���x�x�N�g������A�K�؂Ȓf�ʐς�Ԃ�
	float getCrossArea(glm::vec3 direction) const
	{
		if (glm::length(direction) <= 0.0f)
		{
			return 0.0f;
		}

		direction = glm::normalize(direction);

		for (int i = 0; i < 8; i++)
		{
			float dot = glm::normalizeDot(direction, crossArea[i].first);

			dot = std::abs(dot);

			if (dot > 0.5f)
			{
				return crossArea[i].second;
			}
		}

		return 0.0f;
	}
};

class Force
{
protected:

	//���v�̗�
	glm::vec3 totalForce;

public:

	Force() = default;

	//���̉^���ɂ��͂�Ԃ�
	virtual std::vector<glm::vec3> getForce(const PhysicParam& param, const glm::vec3& initVelocity, const float& deltaTime)
	{
		return std::vector<glm::vec3>();
	}

	//���̉^���ɂ��͂̃��[�����g��Ԃ�
	virtual std::vector<glm::vec3> getMoment(const glm::vec3& rotatePoint, const PhysicParam& param, const glm::vec3& initMoment, const float& deltaTime)
	{
		std::vector<glm::vec3> moment(1);

		moment[0] = glm::vec3(0.0f);

		return moment;
	}
};

//�d�̗͂�
class Gravity : public Force
{
private:

	//�d�͉����x
	glm::vec3 g;

	//��C���x�W��
	float airDensity;

	//�R�͌W��(�����̂�z��)
	float resistanceConstant;

public:

	Gravity()
	{
		g = glm::vec3(0.0f, 9.8f, 0.0f);

		airDensity = 1.293f;

		resistanceConstant = 1.0f;
	}

	Gravity(const glm::vec3& gravity)
	{
		g = gravity;
	}

	std::vector<glm::vec3> getForce(const PhysicParam& param, const glm::vec3& initVelocity, const float& deltaTime) override
	{
		std::vector<glm::vec3> force(1);

		//�d��
		glm::vec3 gravity = param.m * g;

		//��C��R
		glm::vec3 airResistance = resistanceConstant * initVelocity;

		force[0] = gravity - airResistance;

		return force;
	}
};

//�����R��
class CollisionForce : public Force
{
private:

	//�d�͉����x
	glm::vec3 g;

	//�Փˎ��Ɍv�Z���������x�N�g���̌���
	glm::vec3 collisionDirection;

	//�Ԃ������I�u�W�F�N�g�̐Ö��C�͌W��
	float staticFriction;

	//�����C�͌W��
	float kineticFriction;

	//�����W��
	float bounciness;

	//�Փ˓_
	glm::vec3 collisionPoint;

	//�Փ˖@��
	glm::vec3 collisionNormal;

public:

	CollisionForce()
	{
		g = glm::vec3(0.0f, 9.8f, 0.0f);

		staticFriction = 1.0f;
		kineticFriction = 1.0f;
		
		bounciness = 0.0f;
	}

	CollisionForce(const glm::vec3& gravity)
	{
		g = gravity;

		staticFriction = 1.0f;
		kineticFriction = 1.0f;

		bounciness = 0.0f;
	}

	glm::vec3 getTotalForce()
	{
		return totalForce;
	}

	//�Փˎ��̉����x�N�g�����Փ˖ʂ̖@��
	//�Փˑ���̖��C�͌W��
	//��ݒ肷��
	void setCollisionVector(const glm::vec3& normal,const PhysicParam& param)
	{
		collisionDirection = glm::normalize(normal);

		staticFriction = param.staticFriction;
		kineticFriction = param.kineticFriction;
	}

	void setCollisionVector(const glm::vec3& normal,const glm::vec3& point)
	{
		collisionDirection = glm::normalize(normal);

		collisionPoint = point;

		collisionNormal = collisionDirection;
	}

	std::vector<glm::vec3> getForce(const PhysicParam& param, const glm::vec3& initVelocity, const float& deltaTime) override
	{
		//�����R�͂ƐÖ��C�͂̓��
		std::vector<glm::vec3> force(2);

		float top = glm::dot(param.m * g, collisionDirection);
		float bottom = ((glm::length(param.m * g) * glm::length(collisionDirection)));
		
		float cos = top / bottom;
		float sin = sqrt(1.0f - (cos * cos));
		float tan = sin / cos;

		//�����R�͂����߂�
		force[0] = param.m * (-initVelocity);

		//�C���p���X�@���̗p���邽��
		//�����R�͂����ԃX�e�b�v�Ŋ���A������葬�x��0�ɂȂ�悤�ȗ͂ɕς���
		force[0] /= deltaTime;

		//�d�͂̒��́A�Ζʂɕ��s�Ȑ���
		glm::vec3 horizontalForce = param.m * g * sin;

		//�Ζʂɕ��s�ȃx�N�g��
		glm::vec3 horizntal = glm::normalize(horizontalForce);

		//���̂�����o�����ǂ����𒲂ׂ�
		if (tan < staticFriction)
		{
			force[1] = glm::vec3(0.0f);
		}
		else
		{
			//�Ζʂɕ��s�ȗ͂̍��v
			force[1] = horizontalForce - kineticFriction * horizntal;
		}

		totalForce = glm::vec3(0.0f);
		for (int i = 0; i < force.size(); i++)
		{
			totalForce += force[i];
		}

		return force;
	}

	//���̉^���ɂ��͂̃��[�����g��Ԃ�
	std::vector<glm::vec3> getMoment(const glm::vec3& rotatePoint, const PhysicParam& param, const glm::vec3& initMoment, const float& deltaTime) override
	{
		const glm::vec3 line = collisionPoint - rotatePoint;

		std::vector<glm::vec3> morment(1);

		morment[0] = glm::vec3(0.0f);
		if (glm::length(line) >= 0.1f)
		{
			morment[0] = glm::cross(line, totalForce);
		}

		return morment;
	}
};