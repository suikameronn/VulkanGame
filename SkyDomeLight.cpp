#include"FileManager.h"
#include"VulkanBase.h"

#include"SkyDomeLight.h"

SkyDomeLight* SkyDomeLight::skyDomeLight = nullptr;

SkyDomeLight::SkyDomeLight()
{
	objNum = SKYDOMELIGHT;
}

SkyDomeLight::~SkyDomeLight()
{
	VkDevice device = VulkanBase::GetInstance()->getDevice();

	if (envLightMapTexDiffuse)
	{
		envLightMapTexDiffuse->destroy(device);
	}

	if (envLightMapTexSpecular)
	{
		envLightMapTexSpecular->destroy(device);
	}
}

void SkyDomeLight::setEnvLightMapImage(std::string envLightMapImagePath)
{
	envLightMapImage = FileManager::GetInstance()->loadImage(envLightMapImagePath);

	envLightMapTexDiffuse = VulkanBase::GetInstance()->createIBLTexDiffuse(envLightMapImage);
	envLightMapTexDiffuse = VulkanBase::GetInstance()->createIBLTexSpecular(envLightMapImage);
}

std::array<glm::mat4, 6> SkyDomeLight::getViewMatrices()
{
	return {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
}