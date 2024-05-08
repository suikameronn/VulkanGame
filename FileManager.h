#pragma once
#include<iostream>
#include<unordered_map>

#include"Model.h"

struct ImageData
{
	int width;
	int height;
	int texChannels;
	std::shared_ptr<unsigned char> pixcels;
};

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
	std::unordered_map<OBJECT, std::shared_ptr<Model>> models;
	std::string getModelPath(OBJECT obj);

	//�摜�t�@�C���̃p�X������
	std::string imagePath;
	std::unordered_map < IMAGE, std::shared_ptr<ImageData>> images;
	std::string getImagePath(IMAGE image);

public:
	Model* loadModel(OBJECT obj);
	Model* getModelData(OBJECT obj);

	/*
	ImageData* loadImage(IMAGE image);
	ImageData* getImageData(IMAGE image);
	*/
};