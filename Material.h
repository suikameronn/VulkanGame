#pragma once
#include<iostream>
#include<vector>
#include<glm/glm.hpp>

class ImageData
{
private:
	uint32_t width;
	uint32_t height;
	uint32_t texChannels;
	std::vector<unsigned char> pixels;

public:
	ImageData()
	{
		this->width = 0;
		this->height = 0;
		this->texChannels = 0;
	}

	ImageData(int width, int height, int texChannel)
	{
		this->width = static_cast<uint32_t>(width);
		this->height = static_cast<uint32_t>(height);
		this->texChannels = static_cast<uint32_t>(texChannel);
		pixels.resize(width * height * texChannels);
	}

	ImageData(int width, int height,
		int texChannels, unsigned char* pixels)
	{
		this->width = static_cast<uint32_t>(width);
		this->height = static_cast<uint32_t>(height);
		this->texChannels = static_cast<uint32_t>(texChannels);
		this->pixels.resize(width * height * 4);
		this->pixels.assign(pixels, pixels + ((width * height * 4) - 1));
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
		return pixels.data();
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