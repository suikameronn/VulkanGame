#include"Model.h"
#include"VulkanBase.h"

Model::Model()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	rotate.radian = 0.0f;
	rotate.direction = glm::vec3(0.0f, 1.0f, 0.0f);

	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	parentObject = nullptr;
	childObjects.clear();
	spherePos = false;
	theta = 0.0f;
	phi = 0.0f;

	rotateSpeed = 0.1f;
}

void Model::cleanupVulkan()
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

BufferObject* Model::getPointBuffer(uint32_t i)
{
	return &pointBuffers[i];
}

MappedBuffer* Model::getMappedBuffer(uint32_t i)
{
	return &mappedBuffers[i];
}

uint32_t Model::getimageDataCount()
{
	return imageDataCount;
}

glm::mat4 Model::getTransformMatrix()
{
	return transformMatrix;
}

glm::vec3 Model::getAverageLocalPos()
{
	return fbxModel->getAverageLocalPos();
}

void Model::Update()
{

	if (controllable)
	{
		glm::vec3 moveDirec = inputMove();
		setPosition(this->position + moveDirec * speed);
	}

	if (spherePos)
	{
		setSpherePos(theta, phi);
	}
}

void Model::updateTransformMatrix()
{
	transformMatrix = glm::translate(glm::mat4(1.0), position - getAverageLocalPos())
		* glm::rotate(glm::mat4(1.0), rotate.radian, rotate.direction)
		* glm::scale(scale);
}