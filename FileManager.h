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
	//ファイルのパスを入れる
	std::string modelPath;
	std::string getModelPath(OBJECT obj);

	//画像ファイルのパスを入れる
	std::string imagePath;
	std::string getImagePath(IMAGE image);

public:
	Geometry* loadModelPoints(OBJECT obj);
	Geometry* getModelData(OBJECT obj);

	std::shared_ptr<ImageData> loadModelImage(IMAGE image);
	ImageData* getImageData(IMAGE image);
	uint32_t getRegisteredImageCount();
};