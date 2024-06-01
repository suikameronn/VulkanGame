#pragma once
#include<iostream>
#include "fbxsdk.h"

#include"Storage.h"

enum OBJECT;

enum IMAGE;

class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	//ファイルのパスを入れる
	std::string modelPath;
	std::string getModelPath(OBJECT obj);

	//画像ファイルのパスを入れる
	std::string imagePath;
	std::string getImagePath(IMAGE image);
	ImageData* imageData = nullptr;

	FbxManager* manager;
	FbxIOSettings* ios;
	FbxImporter* importer;
	FbxScene* scene;
	FbxGeometryConverter* converter;

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

		delete converter;
		importer->Destroy();
		ios->Destroy();
		manager->Destroy();
	}

	Meshes* loadModelPoints(OBJECT obj);
	Meshes* loadObj(OBJECT obj);
	Meshes* loadPointsFbx(OBJECT obj);
	void loadPointsFbx(Meshes* meshes, FbxNode* node);
	ImageData* loadModelImage(IMAGE image);
};