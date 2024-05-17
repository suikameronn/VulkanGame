#include"VkModel.h"

VkModel::VkModel(Model* model)
{
	this->model = model;
}

VkModel::~VkModel()
{
	vkDestroyBuffer(device, vertBuffer.buffer, nullptr);
	vkFreeMemory(device, vertBuffer.handler, nullptr);

	vkDestroyBuffer(device, indeBuffer.buffer, nullptr);
	vkFreeMemory(device, indeBuffer.handler, nullptr);

	vkDestroySampler(device, textureData.sampler, nullptr);
	vkDestroyImageView(device, textureData.view, nullptr);

	vkDestroyImage(device, textureData.image, nullptr);
	vkFreeMemory(device, textureData.memory, nullptr);
}

void VkModel::setModel(Model* model)
{
	this->model = model;
}

void VkModel::setLayout(VkDescriptorSetLayout* layout)
{
	this->layout = layout;
}

void VkModel::setPipeline(VkPipeline* pipeline)
{
	this->pipeline = pipeline;
}

void VkModel::setPool(VkDescriptorPool* pool)
{
	this->pool = pool;
}

void VkModel::setDescriptorSet(VkDescriptorSet* descriptorSet)
{
	this->descriptorSet = descriptorSet;
}

VkDescriptorSetLayout* VkModel::getLayout()
{
	return layout;
}

VkPipeline* VkModel::getPipeline()
{
	return pipeline;
}

VkDescriptorPool* VkModel::getPool()
{
	return pool;
}

VkDescriptorSet* VkModel::getDescriptorSet()
{
	return descriptorSet;
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

MappedBuffer* VkModel::getMappedBuffer()
{
	return &mappedBuffer;
}

TextureData* VkModel::getTextureData()
{
	return &textureData;
}

BufferObject* VkModel::getVertBuffer()
{
	return &vertBuffer;
}

BufferObject* VkModel::getIndeBuffer()
{
	return &indeBuffer;
}