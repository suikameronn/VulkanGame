#include"Material.h"

Material::Material()
{
	this->diffuse = glm::vec3(1.0,1.0,1.0);
	this->ambient = glm::vec3(1.0, 1.0, 1.0);
	this->specular = glm::vec3(1.0, 1.0, 1.0);
	this->emissive = glm::vec3(1.0, 1.0, 1.0);
	this->shininess = 0.0;
	this->transmissive = glm::vec3(1.0, 1.0, 1.0);

	texture = new ImageData();
}

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

void Material::setImageData(ImageData* image)
{
	texture = image;
}

ImageData* Material::getImageData()
{
	return texture;
}
