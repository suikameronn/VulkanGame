#include"Model.h"
#include"VulkanBase.h"

Model::Model()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = { 0,0,0 };

	rotate.radian = 0.0f;
	rotate.direction = glm::vec3(0.0f, 1.0f, 1.0f);

	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	otherObject = nullptr;
	spherePos = false;
	theta = 0.0f;
	phi = 0.0f;

	theta2 = theta;
	phi2 = phi;

	current = { 1.0, 0.0, 0.0, 0.0 };
	after = { 1.0, 0.0, 0.0, 0.0 };
	target = { 1.0,0.0,0.0,0.0 };
	quatMat = glm::mat4(1.0);

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

std::bitset<8> Model::getLayoutBit()
{
	return layoutBit;
}

VkDescriptorSet* Model::getDescriptorSet(uint32_t i)
{
	return &descriptorSets[i];
}

glm::mat4 Model::getTransformMatrix()
{
	return transformMatrix;
}

void Model::Update()
{
	glm::vec3 pos = { 0,0,0 };


	//setPosition(pos);

	theta += rotateSpeed;
	phi = 30.0f;

	setSpherePos(pos, 5.0f, theta, phi);
}

void Model::updateTransformMatrix()
{
	transformMatrix = glm::translate(glm::mat4(1.0), position)
		* glm::rotate(glm::mat4(1.0), rotate.radian, rotate.direction)
		* glm::scale(scale);
}