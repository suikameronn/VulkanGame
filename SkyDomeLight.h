#pragma once
#include"Light.h"

#include"Material.h"
#include<vulkan/vulkan.h>

class SkyDomeLight : public Light
{
private:
	std::shared_ptr<ImageData> envLightMapImage;
	TextureData* envLightMapTexDiffuse;
	TextureData* envLightMapTexSpecular;

	static SkyDomeLight* skyDomeLight;
	SkyDomeLight();
	~SkyDomeLight();

public:

	std::array<VkImage, 6> cubeMapImages;

	static SkyDomeLight* GetInstance()
	{
		if (!skyDomeLight)
		{
			skyDomeLight = new SkyDomeLight();
		}

		return skyDomeLight;
	}

	static void Destory()
	{
		if (skyDomeLight)
		{
			delete skyDomeLight;
		}
	}

	void setEnvLightMapImage(std::string envLightMapImagePath);

	std::array<glm::mat4, 6> getViewMatrices();
};