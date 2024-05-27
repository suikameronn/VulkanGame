#include"Model.h"
#include"VulkanBase.h"

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
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	vkDestroyBuffer(device, pointBuffer.vertBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.vertHandler, nullptr);

	vkDestroyBuffer(device, pointBuffer.indeBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.indeHandler, nullptr);

	vkDestroyBuffer(device, mappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, mappedBuffer.uniformBufferMemory, nullptr);
	mappedBuffer.uniformBufferMapped = nullptr;

	vkDestroySampler(device, textureData.sampler, nullptr);
	vkDestroyImageView(device, textureData.view, nullptr);
	vkDestroyImage(device, textureData.image, nullptr);
	vkFreeMemory(device, textureData.memory, nullptr);

	delete material;
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
	descriptorInfo = info;
}

void Model::setDescriptorSet(VkDescriptorSet* descriptorSet)
{
	this->descriptorSet = *descriptorSet;
}

DescriptorInfo* Model::getDescriptorInfo()
{
	return descriptorInfo;
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

void Model::Update()
{

}