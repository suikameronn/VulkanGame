#pragma once
#include<iostream>
<<<<<<< HEAD
#include"FbxModel.h"
#include "fbxsdk.h"
=======
>>>>>>> master

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
	ImageData* imageData = nullptr;

<<<<<<< HEAD
	FbxManager* manager;
	FbxIOSettings* ios;
	FbxImporter* importer;
	FbxScene* scene;
	FbxGeometryConverter* converter;

	int indexSize;

	void loadMesh(Meshes* meshes, FbxMesh* node);
=======
	int vertSize = 0;
	int indexSize = 0;

	Assimp::Importer importer;
	Meshes* meshes;
	Material* material;

	void processNode(const aiNode* node, const aiScene* scene, FbxModel* model);
	Meshes* processAiMesh(const aiMesh* node, const aiScene* scene);
>>>>>>> master

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

<<<<<<< HEAD
	FbxModel* loadModel(OBJECT obj);
=======
	std::shared_ptr<FbxModel> loadModel(OBJECT obj);
	ImageData* loadModelImage(std::string filePath);
>>>>>>> master
};