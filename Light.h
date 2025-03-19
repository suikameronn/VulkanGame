#pragma once

#include"Object.h"
#include<vulkan/vulkan.h>

//ポイントライト
class PointLight :public Object
{
public:
	PointLight();

	//光の色
	glm::vec3 color;
};

//平行光源(シャドウマッピング対応済み)
class DirectionalLight:public Object
{
public:
	DirectionalLight();

	//光の色
	glm::vec3 color;
	//光の方向
	glm::vec3 direction;
};