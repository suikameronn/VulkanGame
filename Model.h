#pragma once
#include"Object.h"

#include<memory>

#include"Material.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};


class Model:Object
{
protected:
	glm::vec3 avePosition;
	std::shared_ptr<Material> material;

public:
	void setPosition(std::vector<glm::vec3>* positions);

	glm::vec3* getPosition();

	void setMaterial(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular, glm::vec3* emissive
		, float* shininess, glm::vec3* transmissive);
	std::shared_ptr<Material> getMaterial();

	 virtual void calcNormal() = 0;
};