#include"Light.h"

//�e��ݒ��lua����Scene�N���X���J�n���Đݒ肳���

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