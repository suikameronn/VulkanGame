#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>
#include<memory>

#include<glm/glm.hpp>

#include"FileManager.h"
#include"Controller.h"

class Scene
{
private:
	//ここにシーンのオブジェクトの名前とモデルが保存される
	std::unordered_map<std::string, Model*> sceneSet;

	void parseScene();

	void setModels();
	void setModels(std::string name);

public:
	Scene();

	bool Update();

	Model* getSceneModelData(std::string name);
};