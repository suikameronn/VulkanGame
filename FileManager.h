#pragma once

#include<iostream>
#include <windows.h>
#include<map>
#include "resource1.h"

#include"EnumList.h"
#include"Storage.h"

#include"fbxsdk.h"

enum IMAGE;

class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	int getModelResource(OBJECT obj);

	FbxManager* manager;
	FbxIOSettings* ios;

	uint32_t imageDataCount = 0;

	uint32_t allVertNum;
	glm::vec3 minPos;
	glm::vec3 maxPos;
	glm::vec3 pivot;

	std::vector<OBJECT> loadAnimationFiles;

	Assimp::Importer importer;

	std::string splitFileName(std::string filePath);

	//void processNode(const FbxNode* node, const FbxScene* scene, FbxModel* model);
	void processNode(const FbxNode* node, const FbxScene* scene, FbxModel* model);
	void calcMinMaxVertexPos(glm::vec3 pos);
	Meshes* processFbxMesh(const FbxMesh* mesh, const FbxScene* scene, uint32_t meshNumVertices, FbxModel* model);
	void processMeshBones(const FbxMesh* mesh, uint32_t meshIndex, FbxModel* model,Meshes* meshes);
	void loadSingleBone(const FbxCluster* bone, uint32_t meshIndex, FbxModel* model,Meshes* meshes);
	int getBoneID(const FbxCluster* bone, FbxModel* model);
	int getBoneID(const std::string boneName, FbxModel* model);
	std::shared_ptr<Material> processMaterial(FbxMesh* mesh, const FbxScene* scene);

	//const FbxNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string nodeName);
	void ReadNodeHeirarchy(FbxAnimLayer* layer, FbxNode* node, AnimNode* parentNode,
		unsigned int i, FbxModel* model);
	void ReadNodeHeirarchy(FbxAnimLayer* layer, FbxNode* node, AnimNode* parentNode,
		FbxModel* model, Animation* animation);
	void ReadNodeHeirarchy(FbxScene* scene, const FbxNode* node, std::array<glm::mat4, 250>& matrix,FbxModel* fbxModel);
	void ReadNodeHeirarchy(FbxScene* scene, const FbxNode* node,aiMatrix4x4 matrix, std::array<glm::mat4, 250>& matrixArray,FbxModel* fbxModel);


	void loadPose(const FbxScene* scene, FbxModel* fbxModel);
	void loadAnimation(const FbxScene* scene,FbxModel* model,Animation* animation);

	void loadFbxModel(int id, void** ptr, int& size);

	void loadPoses(FbxModel* fbxModel);
	
	std::string extractFileName(std::string path);
	int getImageID(std::string path);
	std::shared_ptr<ImageData> loadModelImage(std::string filePath);

	glm::mat4 FbxMatrix4x4ToGlm(const FbxAMatrix* from);
	glm::vec3 aiVec3DToGLM(const aiVector3D& vec);

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