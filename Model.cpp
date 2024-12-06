#include"Model.h"
#include"VulkanBase.h"

Model::Model()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	childObjects.clear();
	spherePos = false;
	theta = 0.0f;
	phi = 0.0f;

	playAnim = true;

	rotateSpeed = 0.1f;

	deltaTime = 0.0;

	colider = nullptr;
}

void Model::cleanupVulkan()
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	for (uint32_t i = 0; i < pointBuffers.size(); i++)
	{
		vkDestroyBuffer(device, pointBuffers[i].vertBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].vertHandler, nullptr);

		vkDestroyBuffer(device, pointBuffers[i].indeBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].indeHandler, nullptr);
	}

	for (int i = 0; i < mappedBuffers.size(); i++)
	{
		vkDestroyBuffer(device, mappedBuffers[i].uniformBuffer, nullptr);
		vkFreeMemory(device, mappedBuffers[i].uniformBufferMemory, nullptr);
		mappedBuffers[i].uniformBufferMapped = nullptr;
	}

	if (colider)
	{
		colider->cleanupVulkan();
	}
}

void Model::setgltfModel(std::shared_ptr<GltfModel> model)
{
	gltfModel = model;

	jointMatrices.resize(model->jointNum);
	pointBuffers.resize(model->meshCount);
	mappedBuffers.resize(model->primitiveCount);
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

void Model::bindObject(std::weak_ptr<Object> obj)
{
	if (obj.expired())
	{
#ifdef _DEBUG
		throw std::runtime_error("bindObject(Object* obj): bindObject is nullptr");
#endif

		return;
	}

	childObjects.push_back(obj);

	sendPosToChildren(position);
}

void Model::bindCamera(std::weak_ptr<Object> camera)
{
	cameraObj = camera;

	sendPosToChildren(position);
}

void Model::playAnimation()
{
	if (true)
	{
		if (deltaTime > gltfModel->animationDuration("Running"))
		{
			startTime = clock();
		}

		currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

		gltfModel->updateAnimation(deltaTime,"Running", jointMatrices);
	}
}

std::array<glm::mat4, 128>& Model::getJointMatrices(int index)
{
	return this->jointMatrices[index];
}

void Model::setPosition(glm::vec3 pos)
{
	if (position == pos)
	{
		return;
	}

	position = pos;

	sendPosToChildren(position);

	uniformBufferChange = true;
}

void Model::updateTransformMatrix()
{
	//transformMatrix = pivotMatrix;
	//pivot = glm::vec4(pivot,0.f) * glm::scale(glm::mat4(1.0f), scale);

	transformMatrix = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f),scale);

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = true;
}

void Model::setColider()
{
	colider = std::shared_ptr<Colider>(new Colider(gltfModel->boundingBox.min, gltfModel->boundingBox.max));
}

bool Model::hasColider()
{
	if (colider)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Model::sendPosToChildren(glm::vec3 pos)
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		if (!itr->expired())
		{
			itr->lock()->setParentPos(pos);
		}
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(pos);
	}
}