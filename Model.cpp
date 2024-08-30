#include"Model.h"
#include"VulkanBase.h"

Model::Model()
{
	uniformBufferChange = true;

	pivot = { 0,0,0 };
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

	playAnim = true;

	rotateSpeed = 0.1f;

	deltaTime = 0.0;
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
	pivot = model->getAverageLocalPos();

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
		if (deltaTime > 60.0f)
		{
			startTime = clock();
		}

		currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

		std::cout << deltaTime << std::endl;
	}
}

std::array<glm::mat4,250> Model::getBoneInfoFinalTransform()
{
	if (1)
	{
		//std::cout << deltaTime << std::endl;
		return fbxModel->getAnimationMatrix(deltaTime, playAnimName);
	}

	return fbxModel->getAnimationMatrix();//‰ŠúŽp¨‚Ìƒ{[ƒ“‚ð•Ô‚·
}

void Model::startAnimation(std::string name)
{
	playAnim = true;
	playAnimName = name;

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
	transformMatrix = glm::translate(glm::mat4(1.0), position)
		* glm::rotate(glm::mat4(1.0), rotate.radian, rotate.direction)
		* glm::scale(scale);
}