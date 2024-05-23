#pragma once
#include<iostream>

#include"Storage.h"

enum OBJECT;
enum IMAGE;


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

	void FinishFileManger()
	{
		delete fileManager;
	}

	~FileManager()
	{
		fileManager = nullptr;
	}

	Meshes* loadModelPoints(OBJECT obj);
	ImageData* loadModelImage(IMAGE image);
};