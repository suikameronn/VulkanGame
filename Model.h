#pragma once
#include<vector>
#include"Geometry.h"

//画面などに現れるモデル、それぞれのクラス
//同じモデルでも、固有の名前が付けられている
class Model
{
private:
	std::string name;
	std::shared_ptr<Geometry> geo;

	glm::vec3 pos;
public:
	void setName(std::string& n);
	std::string& getName();
};