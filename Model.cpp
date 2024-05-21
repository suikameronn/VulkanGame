#include"Model.h"

Model::Model()
{
	meshes = nullptr;
	material = nullptr;
}

Model::Model(Meshes* meshes, Material* material)
{
	this->meshes = meshes;
	this->material = material;
}

Model::~Model()
{
	vkDestroyBuffer(VulkanBase::GetInstance()->GetDevice(), vertBuffer.buffer, nullptr);
	vkFreeMemory(VulkanBase::GetInstance()->GetDevice(), vertBuffer.handler, nullptr);

	vkDestroyBuffer(VulkanBase::GetInstance()->GetDevice(), indeBuffer.buffer, nullptr);
	vkFreeMemory(VulkanBase::GetInstance()->GetDevice(), indeBuffer.handler, nullptr);

	vkDestroySampler(VulkanBase::GetInstance()->GetDevice(), textureData.sampler, nullptr);
	vkDestroyImageView(VulkanBase::GetInstance()->GetDevice(), textureData.view, nullptr);

	vkDestroyImage(VulkanBase::GetInstance()->GetDevice(), textureData.image, nullptr);
	vkFreeMemory(VulkanBase::GetInstance()->GetDevice(), textureData.memory, nullptr);
}

void Model::setMeshes(Meshes* meshes)
{
	this->meshes = meshes;
}

void Model::setMaterial(Material* material)
{
	this->material = material;
}

void Model::setImageData(ImageData* image)
{
	if (material == nullptr)
	{
		material = new Material();
	}
	this->material->setImageData(image);
}

Meshes* Model::getMeshes()
{
	return this->meshes;
}

Material* Model::getMaterial()
{
	return this->material;
}

void Model::setDescriptorSet(VkDescriptorSet* descriptorSet)
{
	this->descriptorSet = *descriptorSet;
}

VkDescriptorSet* Model::getDescriptorSet()
{
	return &descriptorSet;
}

MappedBuffer* Model::getMappedBuffer()
{
	return &mappedBuffer;
}

TextureData* Model::getTextureData()
{
	return &textureData;
}

BufferObject* Model::getVertBuffer()
{
	return &vertBuffer;
}

BufferObject* Model::getIndeBuffer()
{
	return &indeBuffer;
}