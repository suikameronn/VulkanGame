#include"Model.h"

void Model::setPosition(std::vector<glm::vec3>* positions)
{
	avePosition = glm::vec3(0.0, 0.0, 0.0);
	for (auto itr = positions->begin(); itr != positions->end(); itr++)
	{
		avePosition += *itr;
	}

	int size = positions->size();
	if (size != 0)
	{
		avePosition /= glm::vec3(size);
	}
}

glm::vec3* Model::getPosition()
{
	return &avePosition;
}

void Model::setMaterial(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular, glm::vec3* emissive
	, float* shininess, glm::vec3* transmissive)
{
	material = std::unique_ptr<Material>(new Material(diffuse, ambient, specular, emissive, shininess, transmissive));
}

std::shared_ptr<Material> Model::getMaterial()
{
	return material;
}