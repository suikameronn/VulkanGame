#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include"FileManager.h"
#include"Geometry.h"
#include"VulkanBase.h"

class Scene
{
private:
	//ここにシーンのオブジェクトの名前とモデルが保存される
	std::unordered_map<std::string, Geometry> sceneSet;

	//シーン全体のモデルへのポインターを持つ
	std::vector<Geometry*> modelsPointer;
	void setPointers();
	void setPointers(std::string name);

public:
	Scene();
	Scene(FileManager* manager);

	Geometry* getSceneModelData(std::string name);
	std::vector<Geometry*>& getSceneModel();
};