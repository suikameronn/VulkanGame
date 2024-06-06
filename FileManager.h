#pragma once
#include<iostream>

#include"Storage.h"

enum OBJECT;

enum IMAGE;

class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	//�t�@�C���̃p�X������
	std::string modelPath;
	std::string getModelPath(OBJECT obj);

	//�摜�t�@�C���̃p�X������
	std::string imagePath;
	std::string getImagePath(IMAGE image);
	ImageData* imageData = nullptr;

	int vertSize = 0;
    int indexSize = 0;

public:
	static FileManager* GetInstance()
	{
		if (!fileManager)
		{
			fileManager = new FileManager();
		}

		return fileManager;
	}

	void FinishFileManger()
	{
		delete fileManager;
	}

	~FileManager()
	{
		fileManager = nullptr;
	}


	ImageData* loadModelImage(IMAGE image);
};