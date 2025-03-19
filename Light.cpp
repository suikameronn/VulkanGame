#include"Light.h"

//各種設定はluaからSceneクラスを開始して設定される

PointLight::PointLight()
{
	objNum = POINTLIGHT;
	color = glm::vec3(1.0f);
}

DirectionalLight::DirectionalLight()
{
	objNum = DIRECTIONALLIGHT;
	direction = glm::vec3(1.0f);
	color = glm::vec3(1.0f);
}