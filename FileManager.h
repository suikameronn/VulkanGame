#pragma once
#include<iostream>
#include<unordered_map>

#include"Model.h"

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
	std::unordered_map<OBJECT, std::shared_ptr<Model>> models;
	std::string getModelPath(OBJECT obj);

	//画像ファイルのパスを入れる
	std::string imagePath;
	std::unordered_map < IMAGE, std::shared_ptr<ImageData>> images;
	std::string getImagePath(IMAGE image);

public:
	Model* loadModelPoints(OBJECT obj);
	Model* getModelData(OBJECT obj);

	ImageData* loadModelImage(IMAGE image);
	ImageData* getImageData(IMAGE image);
};