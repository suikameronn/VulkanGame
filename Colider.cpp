#include"Colider.h"

Colider::Colider(glm::vec3 min,glm::vec3 max)
{
	rightForwardUp = glm::vec3(max.x,max.y,max.z);
	rightForwardDown = glm::vec3(max.x, min.y, max.z);
	rightBackUp = glm::vec3(max.x, max.y, min.z);
	rightBackDown = glm::vec3(max.x, min.y, min.z);

	leftForwardUp = glm::vec3(min.x, max.y, max.z);
	leftForwardDown = glm::vec3(min.x, min.y, max.z);
	leftBackUp = glm::vec3(min.x, max.y, min.z);
	leftBackDown = glm::vec3(min.x, min.y, min.z);

	coliderVertices.resize(12);

	coliderVertices[0] = rightForwardUp - leftForwardUp;
	coliderVertices[1] = rightBackUp - rightForwardUp;
	coliderVertices[2] = leftBackUp - rightBackUp;
	coliderVertices[3] = leftBackUp - leftForwardUp;

	coliderVertices[4] = rightForwardUp - rightForwardDown;
	coliderVertices[5] = rightBackUp - rightBackDown;
	coliderVertices[6] = leftForwardUp - leftForwardDown;
	coliderVertices[7] = leftBackUp - leftBackDown;

	coliderVertices[8] = rightForwardDown - leftForwardDown;
	coliderVertices[9] = rightBackDown - rightForwardDown;
	coliderVertices[10] = leftBackDown - rightBackDown;
	coliderVertices[11] = leftBackDown - leftForwardDown;
}

glm::vec3* Colider::getColiderVertices()
{
	return coliderVertices.data();
}

int Colider::getColiderVerticesSize()
{
	return coliderVertices.size();
}

void Colider::reflectMovement(glm::mat4& transform)
{
	for (auto itr = coliderVertices.begin(); itr != coliderVertices.end(); itr++)
	{
		*itr = glm::vec3(glm::vec4(*itr,1.0f) * transform);
	}
}

Model* Colider::Intersect()
{
	return nullptr;
}