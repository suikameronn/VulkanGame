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
	//ここにシーンのオブジェクトの名前とモデルが保存される
	std::unordered_map<std::string, Model> sceneSet;

	//シーン全体のモデルへのポインターを持つ
	std::vector<Model*> modelsPointer;
	void setPointers();
	void setPointers(std::string name);

public:
	Scene();
	Scene(FileManager* manager);

	Model* getSceneModelData(std::string name);
	std::vector<Model*>& getSceneModel();
};