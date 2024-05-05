#pragma once
#include<iostream>
#include<unordered_map>

#include"Model.h"

enum OBJECT
{
	TEST = 0
};

class FileManager
{
private:
	//�t�@�C���̃p�X������
	std::string path;

	std::unordered_map<OBJECT, std::shared_ptr<Model>> models;

	std::string getPath(OBJECT obj);
public:
	Model* loadModel(OBJECT obj);
	Model* getModelData(OBJECT obj);
};