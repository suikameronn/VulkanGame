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

#include"font.h"

enum IMAGE;

//画像やgltfモデルの読み込みを担当するクラス
class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	//埋め込められたgltfモデルを取得する
	int getModelResource(GLTFOBJECT obj);

	//コライダーの計算用変数
	glm::vec3 minPos;
	glm::vec3 maxPos;
	int vertexNum;
	int indexNum;

	//std::vector<GLTFOBJECT> loadAnimationFiles;

	//与えられた文字列からファイルの名前のみを取得する
	std::string splitFileName(std::string filePath);

	//gltfモデルの読み込みを開始する
	GltfModel* loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel);
	//gltfモデルのノードを再帰的に読み込む
	void loadNode(GltfNode* parent,GltfNode* current, GltfModel* model, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel, float globalscale);
	//gltfモデルのメッシュを読み込む
	void processMesh(const tinygltf::Node& gltfNode, const tinygltf::Model gltfModel, GltfNode* currentNode, GltfModel* model);
	//AABBのためにgltfモデルの頂点のxyzの各軸の最小値と最大値を求める
	void calcMinMaxVertexPos(glm::vec3 min,glm::vec3 max);
	//gltfモデルのプリミティブ単位で読み込む
	void processPrimitive(Mesh* mesh, int& indexStart, tinygltf::Primitive glPrimitive, tinygltf::Model glModel, GltfModel* model);
	//gltfモデルのアニメーションを読み込む
	void loadAnimations(GltfModel* model, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel);
	//gltfモデルのスケルトンを読み込む
	void loadSkin(GltfModel* model, tinygltf::Model gltfModel);
	//自前のModelクラスにスケルトンを設定する
	void setSkin(GltfNode* node, GltfModel* model);

	//テクスチャを読み込む
	void loadTextures(GltfModel* model,const tinygltf::Model gltfModel);
	//マテリアルをメッシュに設定する
	void loadMaterial(GltfModel* model,tinygltf::Model gltfModel);
	//埋め込まれたgltfモデルを取得する
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

	//luaスクリプトからSceneクラスを介して呼び出される。新しいモデルを求められたときのみ、解析処理をする
	std::shared_ptr<GltfModel> loadModel(std::string modelPath);
	//luaスクリプトからSceneクラスを介して呼び出される。新しい画像が求められたときのみ、解析処理をする
	std::shared_ptr<ImageData> loadImage(std::string filePath);
	//
	
};