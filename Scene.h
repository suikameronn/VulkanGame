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
	//ここにシーンのオブジェクトの名前とモデルが保存される
	std::unordered_map<std::string, std::shared_ptr<Model>> sceneSet;

	//シーン全体のモデルへのポインターを持つ
	std::vector<std::shared_ptr<Model>> modelsPointer;
	void setPointers();
	void setPointers(std::string name);

public:
	Scene();
	Scene(std::shared_ptr<FileManager> manager);

	std::shared_ptr<Model> getSceneModelData(std::string name);
	std::vector<std::shared_ptr<Model>>& getSceneModel();
};