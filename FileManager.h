#pragma once
#include<iostream>
#include <windows.h>
#include<map>
#include "resource1.h"
#include<ostream>

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

	const std::string aabbCalc = "Running";
	const int aabbRate = 60;

	FileManager();

	int getModelResource(GLTFOBJECT obj);

	uint32_t imageDataCount = 0;

	uint32_t allVertNum;
	glm::vec3 minPos;
	glm::vec3 maxPos;
	glm::vec3 pivot;

	std::vector<GLTFOBJECT> loadAnimationFiles;

	std::string splitFileName(std::string filePath);

	GltfModel* loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel);
	void loadNode(GltfNode* parent,GltfNode* current, GltfModel* model, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel, float globalscale);
	void processMesh(const tinygltf::Node& gltfNode, const tinygltf::Model gltfModel, GltfNode* currentNode, GltfModel* model);
	void calcMinMaxVertexPos(glm::vec3 min,glm::vec3 max);
	void processPrimitive(Mesh* mesh,int& indexStart,tinygltf::Primitive glPrimitive,tinygltf::Model glModel,GltfModel* model);
	void loadAnimations(GltfModel* model, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel);
	int splitPos(std::string text,std::array<float, 4>& coliderSetting);
	void loadSkin(GltfModel* model, tinygltf::Model gltfModel);
	void setSkin(GltfNode* node, GltfModel* model);

	void loadTextures(GltfModel* model,const tinygltf::Model gltfModel);
	void loadMaterial(GltfModel* model,tinygltf::Model gltfModel);

	void loadgltfModel(int id, void** ptr, int& size);

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

	std::shared_ptr<GltfModel> loadModel(GLTFOBJECT obj);
	std::shared_ptr<ImageData> loadImage(std::string filePath);
	//std::shared_ptr<Animation> loadAnimations(GltfModel* model, OBJECT obj);
};