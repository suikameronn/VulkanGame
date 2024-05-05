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
#include"Model.h"

class Scene
{
private:
	//�����ɃV�[���̃I�u�W�F�N�g�̖��O�ƃ��f�����ۑ������
	std::unordered_map<std::string, std::shared_ptr<Model>> sceneSet;

	//�V�[���S�̂̃��f���ւ̃|�C���^�[������
	std::vector<std::shared_ptr<Model>> modelsPointer;
	void setPointers();
	void setPointers(std::string name);

public:
	Scene();
	Scene(std::shared_ptr<FileManager> manager);

	std::shared_ptr<Model> getSceneModelData(std::string name);
	std::vector<std::shared_ptr<Model>>& getSceneModel();
};