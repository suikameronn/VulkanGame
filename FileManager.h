#pragma once
#include<iostream>
#include"FbxModel.h"
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

	int indexSize;

	void loadMesh(Meshes* meshes, FbxMesh* node);

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

	FbxModel* loadModel(OBJECT obj);
};