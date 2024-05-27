#pragma once
#include"Model.h"

class Player :public Model
{
private:
	int speed;
	int rotateSpeed;

	glm::vec3 forward;

	void inputKeys();

public:
	Player();

	void Update() override;
};