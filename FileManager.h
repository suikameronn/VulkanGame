#pragma once

#include<iostream>
#include <windows.h>
#include<map>
#include "resource1.h"
#include"Storage.h"
#include"Animation.h"

#ifdef _DEBUG
#define	new	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


enum OBJECT;

enum IMAGE;

class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	int getModelResource(OBJECT obj);

	int vertSize = 0;
	int indexSize = 0;

	Assimp::Importer importer;

	void processNode(const aiNode* node, const aiScene* scene, FbxModel* model);
	Meshes* processAiMesh(const aiMesh* node, const aiScene* scene,uint32_t meshIndex, FbxModel* model);
	void processMeshBones(const aiMesh* mesh, uint32_t meshIndex, FbxModel* model);
	void loadSingleBone(const aiBone* bone, uint32_t meshIndex, FbxModel* model);
	int getBoneID(const aiBone* bone, FbxModel* model);
	std::shared_ptr<Material> processAiMaterial(int index, const aiScene* scene);

	const aiNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string nodeName);
	void ReadNodeHeirarchy(const aiScene* scene, aiNode* node, std::shared_ptr<AnimNode> parentNode,
		unsigned int i, FbxModel* model);
	void ReadNodeHeirarchy(const aiScene* scene, aiNode* node, std::shared_ptr<AnimNode> parentNode,
		FbxModel* model);
	void loadAnimation(const aiScene* scene,FbxModel* model);

	void loadFbxModel(int id, void** ptr, int& size);
	
	std::string extractFileName(std::string path);
	int getImageID(std::string path);
	std::shared_ptr<ImageData> loadModelImage(std::string filePath);

	glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from);
	glm::vec3 aiVec3DToGLM(const aiVector3D* vec);

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