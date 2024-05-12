#pragma once
#include<iostream>
#include<unordered_map>

#include"Geometry.h"

enum OBJECT
{
	MODELTEST = 0
};

enum IMAGE
{
	IMAGETEST = 0
};

class FileManager
{
private:
	//�t�@�C���̃p�X������
	std::string modelPath;
	std::string getModelPath(OBJECT obj);

	//�摜�t�@�C���̃p�X������
	std::string imagePath;
	std::string getImagePath(IMAGE image);

public:
	Geometry* loadModelPoints(OBJECT obj);
	Geometry* getModelData(OBJECT obj);

	std::shared_ptr<ImageData> loadModelImage(IMAGE image);
	ImageData* getImageData(IMAGE image);
	uint32_t getRegisteredImageCount();
};