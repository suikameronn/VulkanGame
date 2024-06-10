#include"Material.h"

#include"VulkanBase.h"

Material::Material()
{
	this->diffuse = glm::vec3(1.0,1.0,1.0);
	this->ambient = glm::vec3(1.0, 1.0, 1.0);
	this->specular = glm::vec3(1.0, 1.0, 1.0);
	this->emissive = glm::vec3(1.0, 1.0, 1.0);
	this->shininess = 0.0;
	this->transmissive = glm::vec3(1.0, 1.0, 1.0);

	//texture = new ImageData();
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

Material::~Material()
{
	auto device = VulkanBase::GetInstance()->GetDevice();

	vkDestroySampler(device, textureData->sampler, nullptr);
	vkDestroyImageView(device, textureData->view, nullptr);
	vkDestroyImage(device, textureData->image, nullptr);
	vkFreeMemory(device, textureData->memory, nullptr);
}

void Material::setImageData(std::shared_ptr<ImageData> image)
{
	this->image = image;
	textureData = new TextureData;
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