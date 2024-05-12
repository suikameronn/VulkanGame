#pragma once
#include<iostream>
#include<vector>
#include<glm/glm.hpp>

class ImageData
{
private:
	//同一のテクスチャを参照しているかどうかのためのID
	uint32_t id;
	int width;
	int height;
	int texChannels;
	std::vector<unsigned char> pixels;

public:
	ImageData()
	{
		this->id = -1;
		this->width = 1024;
		this->height = 1024;
		this->texChannels = 4;
		std::fill(pixels.begin(), pixels.end(), 100);
	}

	ImageData(int id, uint32_t width, uint32_t height,
		uint32_t texChannels, std::vector<unsigned char>& pixels)
	{
		this->id = id;
		this->width = width;
		this->height = height;
		this->texChannels = texChannels;
		this->pixels = pixels;
	}

	uint32_t getID()
	{
		return this->id;
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
	std::shared_ptr<ImageData> texture;

public:

	Material();
	Material(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular
		, glm::vec3* emissive, float* shininess, glm::vec3* transmissive);

	int getTextureID()
	{
		if (texture == nullptr)
		{
			return -2;
		}

		return texture->getID();
	}

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

	void setImageData(std::shared_ptr<ImageData> image);
	ImageData* getImageData();
};