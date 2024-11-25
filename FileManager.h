#pragma once
#include<iostream>
#include <windows.h>
#include<map>
#include "resource1.h"

#include"EnumList.h"
#include"Storage.h"

#include<limits>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

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
	glm::vec3 pivot;

	std::vector<OBJECT> loadAnimationFiles;

	std::string splitFileName(std::string filePath);

	GltfModel* loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel);
	void processMesh(const tinygltf::Node& currentNode, const tinygltf::Model gltfModel, GltfModel* model);
	void processMesh(const tinygltf::Node& parentNode, const tinygltf::Node& currentNode, const tinygltf::Model gltfModel, GltfModel* model);
	void calcMinMaxVertexPos(glm::vec3 pos);
	void processPrimitive(Meshes* meshes,int& indexStart,tinygltf::Primitive glPrimitive,tinygltf::Model glModel);
	void processMeshBones(const tinygltf::Scene& scene, const tinygltf::Model gltfModel, GltfModel* model);
	int getBoneID(const std::string boneName, GltfModel* gltfModel);

	//void processMeshBones(const FbxMesh* mesh, uint32_t meshIndex, GltfModel* gltfModel,Meshes* meshes);
	//void loadSingleBone(const FbxCluster* bone, uint32_t meshIndex, GltfModel* gltfModel,Meshes* meshes);
	//int getBoneID(const FbxCluster* bone, GltfModel* gltfModel);
	std::shared_ptr<Material> processMaterial(tinygltf::Model gltfModel,int materialIndex);

	//const FbxNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string nodeName);
	//void ReadNodeHeirarchy(FbxAnimLayer* layer, FbxNode* node, AnimNode* parentNode,
		//unsigned int i, GltfModel* gltfModel);
	//void ReadNodeHeirarchy(FbxAnimLayer* layer, FbxNode* node, AnimNode* parentNode,
		//GltfModel* gltfModel, Animation* animation);
	//void ReadNodeHeirarchy(FbxScene* scene, const FbxNode* node, std::array<glm::mat4, 250>& matrix,GltfModel* model);
	//void ReadNodeHeirarchy(FbxScene* scene, const FbxNode* node,aiMatrix4x4 matrix, std::array<glm::mat4, 250>& matrixArray,GltfModel* model);


	//void loadPose(const FbxScene* scene, GltfModel* model);
	//void loadAnimation(const FbxScene* scene,GltfModel* gltfModel,Animation* animation);

	//void loadFbxModel(int id, void** ptr, int& size);

	//void loadPoses(GltfModel* model);
	
	std::string extractFileName(std::string path);
	int getImageID(std::string path);
	//std::shared_ptr<ImageData> loadModelImage(std::string filePath);

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

	std::shared_ptr<GltfModel> loadModel(OBJECT obj);
	//std::shared_ptr<Animation> loadAnimations(GltfModel* model, OBJECT obj);
};