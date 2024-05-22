#pragma once
#include<iostream>

#include"Storage.h"
#include"Meshes.h"
#include"Material.h"

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
	static FileManager* fileManager;

	FileManager() {};

	//�t�@�C���̃p�X������
	std::string modelPath;
	std::string getModelPath(OBJECT obj);

	//�摜�t�@�C���̃p�X������
	std::string imagePath;
	std::string getImagePath(IMAGE image);
	ImageData* imageData = nullptr;

public:
	static FileManager* GetInstance()
	{
		if (!fileManager)
		{
			fileManager = new FileManager();
		}

		return fileManager;
	}

	~FileManager()
	{
		delete fileManager;
		fileManager = nullptr;
	}

	Meshes* loadModelPoints(OBJECT obj);
	ImageData* loadModelImage(IMAGE image);
};