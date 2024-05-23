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

void Model::setDescriptorInfo(DescriptorInfo* info)
{
	descriptorInfo = *info;
}

void Model::setDescriptorSet(VkDescriptorSet* descriptorSet)
{
	this->descriptorSet = *descriptorSet;
}

DescriptorInfo* Model::getDescriptorInfo()
{
	return &descriptorInfo;
}

BufferObject* Model::getPointBuffer()
{
	return &pointBuffer;
}

MappedBuffer* Model::getMappedBuffer()
{
	return &mappedBuffer;
}

TextureData* Model::getTextureData()
{
	return &textureData;
}

std::bitset<8> Model::getLayoutBit()
{
	return layoutBit;
}

VkDescriptorSet* Model::getDescriptorSet()
{
	return &descriptorSet;
}