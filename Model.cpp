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

	if (colider)
	{
		colider->cleanupVulkan();
	}
}

void Model::setFbxModel(std::shared_ptr<FbxModel> model)
{
	fbxModel = model;

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
	if (fbxModel->animationNum() > 0)
	{
		//std::cout << deltaTime << std::endl;
		return fbxModel->getAnimationMatrix(deltaTime, action);
	}

	return fbxModel->getAnimationMatrix();//�����p���̃{�[����Ԃ�
}

void Model::startAnimation()
{
	playAnim = true;

	startTime = clock();
}

void Model::Update()
{
	if (fbxModel->animationNum() > 0)
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
	//pivot = glm::vec4(pivot,0.f) * glm::scale(glm::mat4(1.0f), scale);
	
	transformMatrix = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f),scale);

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;
}

void Model::changeAction(ACTION act)
{
	action = act;
}

void Model::changeAction()
{
	action = defaultAction;
}

glm::vec3 Model::inputMove()
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (!cameraObj.expired())
	{
		forward = cameraObj.lock()->forward;
		right = cameraObj.lock()->right;
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
		changeAction();
	}

	if (moveDirec != glm::vec3(0.0f))
	{
		moveDirec = glm::normalize(moveDirec);
	}

	return moveDirec * 0.01f;
}

void Model::setColider(COLIDER shape, float right, float left, float top, float bottom, float front, float back)
{
	switch (shape)
	{
	case BOX:
		colider = std::shared_ptr<Colider>(new Colider(position,right,left,top,bottom,front,back));
	}
}

void Model::setColider(COLIDER shape)
{
	glm::vec3 min, max;
	fbxModel->getMinMaxVertexPos(min, max);
	switch (shape)
	{
	case BOX:
		colider = std::shared_ptr<Colider>(new Colider(position, min, max));
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