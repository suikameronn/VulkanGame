#pragma once

#include<iostream>
#include <windows.h>
#include<map>
#include "resource1.h"

#include"EnumList.h"
#include"Storage.h"

#include"Assimp/Base64.hpp"

enum IMAGE;

class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	int getModelResource(OBJECT obj);

	uint32_t imageDataCount = 0;

	uint32_t allVertNum;
	glm::vec3 minPos;
	glm::vec3 maxPos;

	std::vector<OBJECT> loadAnimationFiles;

	Assimp::Importer importer;

	std::string splitFileName(std::string filePath);

	//void processNode(const aiNode* node, const aiScene* scene, FbxModel* model);
	void processNode(/*const aiScene* scene, FbxModel* model*/const aiNode* node, const aiScene* scene, FbxModel* model);
	void calcMinMaxVertexPos(glm::vec3 pos);
	Meshes* processAiMesh(const aiMesh* mesh, const aiScene* scene, uint32_t meshNumVertices, FbxModel* model);
	void processMeshBones(const aiMesh* mesh, uint32_t meshIndex, FbxModel* model,Meshes* meshes);
	void loadSingleBone(const aiBone* bone, uint32_t meshIndex, FbxModel* model,Meshes* meshes);
	int getBoneID(const aiBone* bone, FbxModel* model);
	int getBoneID(const std::string boneName, FbxModel* model);
	std::shared_ptr<Material> processAiMaterial(int index, const aiScene* scene);

	const aiNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string nodeName);
	void ReadNodeHeirarchy(const aiScene* scene, aiNode* node, AnimNode* parentNode,
		unsigned int i, FbxModel* model);
	void ReadNodeHeirarchy(const aiScene* scene, aiNode* node, AnimNode* parentNode,
		FbxModel* model, Animation* animation);
	void ReadNodeHeirarchy(const aiScene* scene, const aiNode* node, std::array<glm::mat4, 250>& matrix,FbxModel* fbxModel);
	void ReadNodeHeirarchy(const aiScene* scene, const aiNode* node,aiMatrix4x4 matrix, std::array<glm::mat4, 250>& matrixArray,FbxModel* fbxModel);


	void loadPose(const aiScene* scene, FbxModel* fbxModel);
	void loadAnimation(const aiScene* scene,FbxModel* model,Animation* animation);

	void loadFbxModel(int id, void** ptr, int& size);

	void loadPoses(FbxModel* fbxModel);
	
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
	std::shared_ptr<Animation> loadAnimations(FbxModel* fbxModel, OBJECT obj);
};