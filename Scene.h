#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include"FileManager.h"

class Scene
{
private:
	//�����ɃV�[���̃I�u�W�F�N�g�̖��O�ƃ��f�����ۑ������
	std::unordered_map<std::string, Model*> sceneSet;

	void parseScene();

	void setModels();
	void setModels(std::string name);

public:
	Scene();

	Model* getSceneModelData(std::string name);
};