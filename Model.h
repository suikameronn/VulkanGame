#pragma once
#include<vector>
#include"Geometry.h"

//��ʂȂǂɌ���郂�f���A���ꂼ��̃N���X
//�������f���ł��A�ŗL�̖��O���t�����Ă���
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