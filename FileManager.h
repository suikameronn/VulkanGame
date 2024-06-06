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

	//ファイルのパスを入れる
	std::string modelPath;
	std::string getModelPath(OBJECT obj);

	//画像ファイルのパスを入れる
	std::string imagePath;
	ImageData* imageData = nullptr;

	int vertSize = 0;
    int indexSize = 0;

	Assimp::Importer importer;
	void processNode(const aiNode* node,const aiScene* scene);
	Meshes* processAiMesh(const aiMesh* node, const aiScene* scene);

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

	std::shared_ptr<FbxModel> loadModel(OBJECT obj);
	ImageData* loadModelImage(std::string filePath);
};