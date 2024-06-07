#include"Model.h"
#include"VulkanBase.h"

Model::Model()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = { 0,0,0 };

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	otherObject = nullptr;
	spherePos = false;
	theta = 0.1f;
	phi = 0.1f;
	rotateSpeed = 0.1f;
}

Model::~Model()
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	for (uint32_t i = 0; i < getMeshesSize(); i++)
	{
		vkDestroyBuffer(device, pointBuffers[i].vertBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].vertHandler, nullptr);

		vkDestroyBuffer(device, pointBuffers[i].indeBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].indeHandler, nullptr);

		vkDestroyBuffer(device, mappedBuffers[i].uniformBuffer, nullptr);
		vkFreeMemory(device, mappedBuffers[i].uniformBufferMemory, nullptr);
		mappedBuffers[i].uniformBufferMapped = nullptr;
	}

	for (uint32_t i = 0; i < getSize<std::vector<TextureData>>(textureDatas); i++)
	{
		vkDestroySampler(device, textureDatas[i].sampler, nullptr);
		vkDestroyImageView(device, textureDatas[i].view, nullptr);
		vkDestroyImage(device, textureDatas[i].image, nullptr);
		vkFreeMemory(device, textureDatas[i].memory, nullptr);
	}

	//delete material;
}

void Model::setFbxModel(std::shared_ptr<FbxModel> model)
{
	fbxModel = model;

	pointBuffers.resize(model->getMeshesSize());
	mappedBuffers.resize(model->getMeshesSize());
}

std::shared_ptr<Meshes> Model::getMeshes(uint32_t i)
{
	return this->fbxModel->getMeshes(i);
}

uint32_t Model::getMeshesSize()
{
	return this->fbxModel->getMeshesSize();
}

void Model::setDescriptorInfo(DescriptorInfo* info)
{
	descriptorInfos.push_back(info);
}

void Model::setDescriptorSet(VkDescriptorSet* descriptorSet)
{
	this->descriptorSets.push_back(*descriptorSet);
}

DescriptorInfo* Model::getDescriptorInfo(uint32_t i)
{
	return descriptorInfos[i];
}

BufferObject* Model::getPointBuffer(uint32_t i)
{
	return &pointBuffers[i];
}

MappedBuffer* Model::getMappedBuffer(uint32_t i)
{
	return &mappedBuffers[i];
}

TextureData* Model::getTextureData(uint32_t i)
{
	return &textureDatas[i];
}

std::bitset<8> Model::getLayoutBit()
{
	return layoutBit;
}

VkDescriptorSet* Model::getDescriptorSet(uint32_t i)
{
	return &descriptorSets[i];
}

void Model::datasResize()
{
	uint32_t size = getMeshesSize();

	//descriptorInfos.resize(size);
	//descriptorSets.resize(size);
}

void Model::Update()
{
	glm::vec3 pos = { 0,0,0 };


	//setPosition(pos);

	glm::vec3 offset = { 0.0f,0.0f,10.0f};
	theta += rotateSpeed;
	phi = 30.0f;

	setSpherePos(pos, 5.0f, theta, phi);
}