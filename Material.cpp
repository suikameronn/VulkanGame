#include"Material.h"

Material::Material()
{
	this->diffuse = glm::vec3(1.0,1.0,1.0);
	this->ambient = glm::vec3(1.0, 1.0, 1.0);
	this->specular = glm::vec3(1.0, 1.0, 1.0);
	this->emissive = glm::vec3(1.0, 1.0, 1.0);
	this->shininess = 0.0f;
	this->transmissive = 0.0f;

	imageDataCount = 0;
}

Material::Material(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular
	, glm::vec3* emissive, float* shininess, float* transmissive)
{
	this->diffuse = *diffuse;
	this->ambient = *ambient;
	this->specular = *specular;
	this->emissive = *emissive;
	this->shininess = *shininess;
	this->transmissive = *transmissive;
}

Material::~Material()
{

}

void Material::setImageData(int uvIndex,std::shared_ptr<ImageData> image)
{
	imageDataCount++;
	this->uvIndex = uvIndex;
	this->image = image;
	textureData = new TextureData;
}

void Material::setMetallicRoughnessMap(int uvIndex, std::shared_ptr<ImageData> map)
{
	imageDataCount++;
	//this->uv
}

std::shared_ptr<ImageData> Material::getImageData()
{
	return image;
}

bool Material::hasImageData()
{
	if (image != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Material::hasTextureData()
{
	if (textureData)
	{
		return true;
	}
	else
	{
		return false;
	}
}