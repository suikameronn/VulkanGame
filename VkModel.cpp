#include"VkModel.h"

VkModel::VkModel(Model* model)
{
	this->model = model;
	this->texDescriptorID = -1;
	this->uniformDescriptorID = -1;
}

void VkModel::setModel(Model* model)
{
	this->model = model;
}

void VkModel::setTexDescriptorID(uint32_t id)
{
	this->texDescriptorID = id;
}

void VkModel::setUniformDescriptorID(uint32_t id)
{
	this->uniformDescriptorID = id;
}

Meshes* VkModel::getMeshes()
{
	return model->getMeshes();
}

Material* VkModel::getMaterial()
{
	return model->getMaterial();
}

ImageData* VkModel::getImageData()
{
	return model->getMaterial()->getImageData();
}