#pragma once
#include<iostream>
#include <windows.h>
#include "resource1.h"
#include"Storage.h"

enum OBJECT;

enum IMAGE;

class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	int getModelResource(std::string path);

	int vertSize = 0;
	int indexSize = 0;

	Assimp::Importer importer;

	void processNode(const aiNode* node, const aiScene* scene, FbxModel* model);
	Meshes* processAiMesh(const aiMesh* node, const aiScene* scene);
	std::shared_ptr<Material> processAiMaterial(int index, const aiScene* scene);

	void loadFbxModel(std::string filePath, void** ptr, int& size);
	std::shared_ptr<ImageData> loadModelImage(std::string filePath);

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
};