#pragma once

#include<lua.hpp>

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
#include"EnumList.h"

class Scene
{
private:
	//ここにシーンのオブジェクトの名前とモデルが保存される
	std::unordered_map<std::string, std::shared_ptr<Model>> sceneSet;

	std::shared_ptr<Camera> camera;

	void parseScene();

	void setModels();
	void setModels(std::string name);

public:
	Scene();

	bool UpdateScene();

	std::shared_ptr<Model> getSceneModelData(std::string name);
};