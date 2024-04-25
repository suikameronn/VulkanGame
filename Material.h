#pragma once
#include<iostream>

#include<glm/glm.hpp>

class Material
{
private:
	glm::vec3 diffuse;
	glm::vec3 ambient;
	glm::vec3 specular;
	glm::vec3 emissive;
	float shininess;
	glm::vec3 transmissive;

public:

	Material() {};
	Material(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular
		, glm::vec3* emissive, float* shininess, glm::vec3* transmissive);

	glm::vec3 getDiffuse() { return diffuse; }
	glm::vec3 getAmbient() { return ambient; }
	glm::vec3 getSpecular() { return specular; }
	glm::vec3 getEmissive() { return emissive; }
	float getShininess() { return shininess; }
	glm::vec3 getTransmissive() { return transmissive; }
};