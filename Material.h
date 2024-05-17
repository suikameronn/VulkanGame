#pragma once
#include<iostream>
#include<vector>
#include<glm/glm.hpp>

class ImageData
{
private:
	int width;
	int height;
	int texChannels;
	std::vector<unsigned char> pixels;

public:
	ImageData()
	{
		this->width = 1024;
		this->height = 1024;
		this->texChannels = 4;
		pixels.resize(width * height * texChannels);
		std::fill(pixels.begin(), pixels.end(), 100);
	}

	ImageData(uint32_t width, uint32_t height,
		uint32_t texChannels, std::vector<unsigned char>& pixels)
	{
		this->width = width;
		this->height = height;
		this->texChannels = texChannels;
		this->pixels = pixels;
	}

	int getWidth()
	{
		return this->width;
	}

	int getHeight()
	{
		return this->height;
	}

	int getTexChannels()
	{
		return this->texChannels;
	}

	unsigned char* getPixelsData()
	{
		return this->pixels.data();
	}
};

class Material
{
private:

	glm::vec3 diffuse;
	glm::vec3 ambient;
	glm::vec3 specular;
	glm::vec3 emissive;
	float shininess;
	glm::vec3 transmissive;

	//テクスチャのもととなる画像データへのポインタ
	ImageData* texture = nullptr;

public:

	Material();
	Material(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular
		, glm::vec3* emissive, float* shininess, glm::vec3* transmissive);

	void setDiffuse(glm::vec3* diffuse) { this->diffuse = *diffuse; }
	void setAmbient(glm::vec3* ambient) { this->ambient = *ambient; }
	void setSpecular(glm::vec3* specular) { this->specular = *specular; }
	void setEmissive(glm::vec3* emissive) { this->emissive = *emissive; }
	void setShininess(float* shininess) { this->shininess = *shininess; }
	void setTransmissive(glm::vec3* transmissive) { this->transmissive = *transmissive; }

	glm::vec3 getDiffuse() { return diffuse; }
	glm::vec3 getAmbient() { return ambient; }
	glm::vec3 getSpecular() { return specular; }
	glm::vec3 getEmissive() { return emissive; }
	float getShininess() { return shininess; }
	glm::vec3 getTransmissive() { return transmissive; }

	void setImageData(ImageData* image);
	ImageData* getImageData();
};