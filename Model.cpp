#include"Model.h"
#include"VulkanBase.h"

Model::Model()
{
	uniformBufferChange = true;

	pivot = { 0,0,0 };
	position = { 0,0,0 };
	posOffSet = 0.0f;

	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	parentObject = nullptr;
	childObjects.clear();
	spherePos = false;
	theta = 0.0f;
	phi = 0.0f;

	playAnim = true;

	rotateSpeed = 0.1f;

	deltaTime = 0.0;

	action = ACTION::IDLE;

	colider = nullptr;
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
	pivot = model->getPivot();
	pivotMatrix = glm::translate(-pivot);

	if (fbxModel->animationNum() > 0)
	{
		pointBuffers.resize(model->getMeshesSize() + fbxModel->getTotalVertexNum());
		mappedBuffers.resize(model->getMeshesSize() + fbxModel->getTotalVertexNum());
	}
	else
	{
		pointBuffers.resize(model->getMeshesSize());
		mappedBuffers.resize(model->getMeshesSize());
	}
}

void Model::setAnimation(std::shared_ptr<FbxModel> model, std::string fileName, ACTION action)
{

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

void Model::playAnimation()
{
	if (true)
	{
		if (deltaTime > 40.0f)
		{
			startTime = clock();
		}

		currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;
	}
}

std::array<glm::mat4,250> Model::getBoneInfoFinalTransform()
{
	if (1)
	{
		//std::cout << deltaTime << std::endl;
		return fbxModel->getAnimationMatrix(deltaTime, action);
	}

	return fbxModel->getAnimationMatrix();//‰ŠúŽp¨‚Ìƒ{[ƒ“‚ð•Ô‚·
}

void Model::startAnimation()
{
	playAnim = true;

	startTime = clock();
}

void Model::Update()
{
	if (true)
	{
		playAnimation();
	}

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
	//transformMatrix = pivotMatrix;
	transformMatrix = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f),scale);
}

void Model::changeAction(ACTION act)
{
	action = act;
}

glm::vec3 Model::inputMove()
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (bindCamera)
	{
		forward = bindCamera->forward;
		right = bindCamera->right;
	}

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = -forward;
		changeAction(ACTION::WALK);
		rotate.y = 90.0f;
	}
	else if (controller->getKey(GLFW_KEY_A))
	{
		moveDirec = -right;
		changeAction(ACTION::WALK);
		rotate.y = 0.0f;
	}
	else if (controller->getKey(GLFW_KEY_D))
	{
		moveDirec = right;
		changeAction(ACTION::WALK);
		rotate.y = 180.0f;
	}
	else if (controller->getKey(GLFW_KEY_S))
	{
		moveDirec = forward;
		changeAction(ACTION::WALK);
		rotate.y = 270.0f;
	}
	else
	{
		moveDirec = { 0,0,0 };
		changeAction(ACTION::IDLE);
	}

	return moveDirec;
}

void Model::setColider(COLIDER col)
{
	glm::vec3 min, max;
	switch (col)
	{
	case BOX:
		fbxModel->getMinMaxVertexPos(min, max);
		colider = std::shared_ptr<Colider>(new Colider(min, max));
	}
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