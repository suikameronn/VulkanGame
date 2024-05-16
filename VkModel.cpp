#include"VkModel.h"

VkModel::VkModel(Model* model)
{
	if (model == nullptr)
	{
		std::runtime_error("VkModel(Model* model): Model is nullptr");
	}
	this->model = model;
	layoutBit.set(0);
}

VkModel::~VkModel()
{
	vkDestroyBuffer(VulkanBase::GetInstance()->getDevice(), vertBuffer, nullptr);
	vkFreeMemory(VulkanBase::GetInstance()->getDevice(), vertHandler, nullptr);

	vkDestroyBuffer(VulkanBase::GetInstance()->getDevice(), indiBuffer, nullptr);
	vkFreeMemory(VulkanBase::GetInstance()->getDevice(), indiHandler, nullptr);

	vkDestroySampler(VulkanBase::GetInstance()->getDevice(), sampler, nullptr);
	vkDestroyImageView(VulkanBase::GetInstance()->getDevice(), view, nullptr);

	vkDestroyImage(VulkanBase::GetInstance()->getDevice(), image, nullptr);
	vkFreeMemory(VulkanBase::GetInstance()->getDevice(), memory, nullptr);
}

void VkModel::setModel(Model* model)
{
	if (model == nullptr)
	{
		std::runtime_error("setModel(Model* model): Model is nullptr");
	}
	this->model = model;

	if (model->getMaterial()->getImageData())
	{
		layoutBit.set(1);
	}
}

void VkModel::setPipeline(VkPipeline* p)
{
	if (p == nullptr)
	{
		std::runtime_error("setPipeline(VkPipeline* p): VkPipeline is nullptr");
	}
	this->pipeline = p;
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

std::pair<std::bitset<8>, ImageData*> VkModel::getPairLayoutImage()
{
	return std::pair<std::bitset<8>, ImageData*>(layout, this->getImageData());
}

VkImageView VkModel::getImageView()
{
	return view;
}

VkSampler VkModel::getSampler()
{
	return sampler;
}