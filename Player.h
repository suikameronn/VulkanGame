#pragma once
#include"Model.h"
#include"Controller.h"

class Player
{
private:
	std::shared_ptr<Object> object;
	float speed;
	int rotateSpeed;

	glm::vec3 moveDirec;

	glm::vec3 forward;
	glm::vec3 right;

	void inputKeys();

public:
	Player();
	void setObject(Model* obj);
	Object* getObject();

	void Update();
};