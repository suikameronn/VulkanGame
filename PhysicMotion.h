#pragma once

#include<vector>

//摩擦力などの物理的特性
struct PhysicParam
{
	//質量
	float m;

	//静摩擦力係数
	float staticFriction;
	//動摩擦力係数
	float kineticFriction;

	//反発力
	float restitution;

	//立方体のコライダーの断面積
	std::array<std::pair<glm::vec3, float>, 8> crossArea;

	PhysicParam()
	{
		m = 1.0f;

		staticFriction = 0.0f;
		kineticFriction = 0.0f;

		restitution = 0.0f;
	}

	//速度ベクトルから、適切な断面積を返す
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

	//合計の力
	glm::vec3 totalForce;

public:

	Force() = default;

	//その運動による力を返す
	virtual std::vector<glm::vec3> getForce(const PhysicParam& param, const glm::vec3& initVelocity, const float& deltaTime)
	{
		return std::vector<glm::vec3>();
	}

	//その運動による力のモーメントを返す
	virtual std::vector<glm::vec3> getMoment(const glm::vec3& rotatePoint, const PhysicParam& param, const glm::vec3& initMoment, const float& deltaTime)
	{
		std::vector<glm::vec3> moment(1);

		moment[0] = glm::vec3(0.0f);

		return moment;
	}
};

//重力の力
class Gravity : public Force
{
private:

	//重力加速度
	glm::vec3 g;

	//空気密度係数
	float airDensity;

	//抗力係数(立方体を想定)
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

		//重力
		glm::vec3 gravity = param.m * g;

		//空気抵抗
		glm::vec3 airResistance = resistanceConstant * initVelocity;

		force[0] = gravity - airResistance;

		return force;
	}
};

//垂直抗力
class CollisionForce : public Force
{
private:

	//重力加速度
	glm::vec3 g;

	//衝突時に計算される解消ベクトルの向き
	glm::vec3 collisionDirection;

	//ぶつかったオブジェクトの静摩擦力係数
	float staticFriction;

	//動摩擦力係数
	float kineticFriction;

	//反発係数
	float bounciness;

	//衝突点
	glm::vec3 collisionPoint;

	//衝突法線
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

	//衝突時の解消ベクトルを衝突面の法線
	//衝突相手の摩擦力係数
	//を設定する
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
		//垂直抗力と静摩擦力の二つ
		std::vector<glm::vec3> force(2);

		float top = glm::dot(param.m * g, collisionDirection);
		float bottom = ((glm::length(param.m * g) * glm::length(collisionDirection)));
		
		float cos = top / bottom;
		float sin = sqrt(1.0f - (cos * cos));
		float tan = sin / cos;

		//垂直抗力を求める
		force[0] = param.m * (-initVelocity);

		//インパルス法を採用するため
		//垂直抗力を時間ステップで割り、無理やり速度が0になるような力に変える
		force[0] /= deltaTime;

		//重力の中の、斜面に平行な成分
		glm::vec3 horizontalForce = param.m * g * sin;

		//斜面に平行なベクトル
		glm::vec3 horizntal = glm::normalize(horizontalForce);

		//物体が滑り出すかどうかを調べる
		if (tan < staticFriction)
		{
			force[1] = glm::vec3(0.0f);
		}
		else
		{
			//斜面に平行な力の合計
			force[1] = horizontalForce - kineticFriction * horizntal;
		}

		totalForce = glm::vec3(0.0f);
		for (int i = 0; i < force.size(); i++)
		{
			totalForce += force[i];
		}

		return force;
	}

	//その運動による力のモーメントを返す
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