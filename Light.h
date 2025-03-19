#pragma once

#include"Object.h"
#include<vulkan/vulkan.h>

//�|�C���g���C�g
class PointLight :public Object
{
public:
	PointLight();

	//���̐F
	glm::vec3 color;
};

//���s����(�V���h�E�}�b�s���O�Ή��ς�)
class DirectionalLight:public Object
{
public:
	DirectionalLight();

	//���̐F
	glm::vec3 color;
	//���̕���
	glm::vec3 direction;
};