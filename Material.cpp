#include"Material.h"

Material::Material(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular
	, glm::vec3* emissive, float* shininess, glm::vec3* transmissive)
{
	this->diffuse = *diffuse;
	this->ambient = *ambient;
	this->specular = *specular;
	this->emissive = *emissive;
	this->shininess = *shininess;
	this->transmissive = *transmissive;
}

