#pragma once
#include<iostream>

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

	//ファイルのパスを入れる
	std::string modelPath;
	std::string getModelPath(OBJECT obj);

	//画像ファイルのパスを入れる
	std::string imagePath;
	std::string getImagePath(IMAGE image);

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