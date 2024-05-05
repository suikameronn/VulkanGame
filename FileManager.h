#pragma once
#include<iostream>
#include<unordered_map>

#include"Scene.h"
#include"Model.h"

enum OBJECT
{
	TEST = 0
};

class FileManager
{
private:
	//ファイルのパスを入れる
	std::string path;

	std::unordered_map<OBJECT, std::shared_ptr<Model>> models;
	std::shared_ptr<Scene> scene;

	std::string getPath(OBJECT obj);
public:
	std::shared_ptr<Model> loadModel(OBJECT obj);
	std::shared_ptr<Model> getModelData(OBJECT obj);
	void loadScene();

	std::shared_ptr<Scene> getScene();
};