#pragma once
#include"Model.h"
#include"Controller.h"

class Player
{
private:
	Model* model;
	float speed;
	int rotateSpeed;

	glm::vec3 moveDirec;

	glm::vec3 forward;
	glm::vec3 right;

	void inputKeys();

public:
	Player();
	void setObject(Model* obj);
	Model* getModel();

	void Update();
};