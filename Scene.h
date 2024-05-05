#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include"FileManager.h"
#include"Model.h"
#include"VulkanBase.h"

class Scene
{
private:
	//�����ɃV�[���̃I�u�W�F�N�g�̖��O�ƃ��f�����ۑ������
	std::unordered_map<std::string, Model> sceneSet;

	//�V�[���S�̂̃��f���ւ̃|�C���^�[������
	std::vector<Model*> modelsPointer;
	void setPointers();
	void setPointers(std::string name);

public:
	Scene();
	Scene(FileManager* manager);

	Model* getSceneModelData(std::string name);
	std::vector<Model*>& getSceneModel();
};