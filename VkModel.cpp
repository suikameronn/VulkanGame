#include"VkModel.h"

VkModel::VkModel(Model* model)
{
	this->model = model;
	layoutBit.set(0);
}

VkModel::~VkModel()
{
	vkDestroyBuffer(device, vertBuffer, nullptr);
	vkFreeMemory(device, vertHandler, nullptr);

	vkDestroyBuffer(device, indiBuffer, nullptr);
	vkFreeMemory(device, indiHandler, nullptr);

	vkDestroySampler(device, sampler, nullptr);
	vkDestroyImageView(device, view, nullptr);

	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, memory, nullptr);
}

void VkModel::setModel(Model* model)
{
	this->model = model;

	if (model->getMaterial()->getImageData())
	{
		layoutBit.set(1);
	}
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

std::bitset<8> VkModel::getLayoutBit()
{
	return layoutBit;
}

std::pair<std::bitset<8>, ImageData*> VkModel::getPairLayoutImage()
{
	return std::pair<std::bitset<8>, ImageData*>(layoutBit, this->getImageData());
}

void VkModel::setDescriptorSetLayout(VkDescriptorSetLayout* layout)
{
	this->layout = layout;
}

void VkModel::setDescriptorSet(std::vector<VkDescriptorSet>* descriptorSet)
{
	this->descriptorSet = descriptorSet;
}