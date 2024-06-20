#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include"FileManager.h"
#include"Controller.h"
#include"Player.h"
#include"Camera.h"

class Scene
{
private:
	//�����ɃV�[���̃I�u�W�F�N�g�̖��O�ƃ��f�����ۑ������
	std::unordered_map<std::string, Object*> sceneSet;

	std::unique_ptr<Camera> camera;

	void parseScene();

	void setModels();
	void setModels(std::string name);

public:
	Scene();

	bool UpdateScene();

	Model* getSceneModelData(std::string name);
};