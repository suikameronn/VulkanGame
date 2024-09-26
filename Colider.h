#pragma once

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include<memory>
#include<vector>

#include"EnumList.h"

class Model;

class Colider
{
private:
	glm::vec3 rightForwardUp;
	glm::vec3 rightForwardDown;
	glm::vec3 rightBackUp;
	glm::vec3 rightBackDown;
	glm::vec3 leftForwardUp;
	glm::vec3 leftForwardDown;
	glm::vec3 leftBackUp;
	glm::vec3 leftBackDown;

	std::shared_ptr<Model> model;
	std::vector<glm::vec3> coliderVertices;

public:
	Colider(glm::vec3 min,glm::vec3 max);

	void reflectMovement(glm::mat4& transform);

	virtual Model* Intersect();
	glm::vec3* getColiderVertices();
	int getColiderVerticesSize();
};