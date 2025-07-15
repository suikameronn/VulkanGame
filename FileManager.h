#pragma once
#include<iostream>
#include <windows.h>
#include<map>
#include<ostream>
#include<thread>
#include<time.h>

#include "resource1.h"

#include"EnumList.h"
#include"Storage.h"
#include"ThreadPool.h"

#include<limits>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include"StructList.h"

enum IMAGE;

//画像やgltfモデルの読み込みを担当するクラス
class FileManager
{
private:
	static FileManager* fileManager;

	//スクリプトから指定された3Dモデルのファイルのリストと
	//それを持たせるModelクラス
	std::unordered_map<std::string,std::vector<Model*>> loadModelList;

	FileManager();

	//コライダーの計算用変数
	glm::vec3 minPos;
	glm::vec3 maxPos;
	int vertexNum;
	int indexNum;

	//与えられた文字列からファイルの名前のみを取得する
	std::string splitFileName(std::string filePath);

	//gltfモデルの読み込みを開始する
	std::shared_ptr<GltfModel> loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel);
	//gltfモデルのノードを再帰的に読み込む
	void loadNode(GltfNode* parent,GltfNode* current, std::shared_ptr<GltfModel> model, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel, float globalscale);
	//gltfモデルのメッシュを読み込む
	void processMesh(const tinygltf::Node& gltfNode, const tinygltf::Model gltfModel, GltfNode* currentNode, std::shared_ptr<GltfModel> model,int meshIndex);
	//AABBのためにgltfモデルの頂点のxyzの各軸の最小値と最大値を求める
	void calcMinMaxVertexPos(glm::vec3 min,glm::vec3 max);
	//gltfモデルのプリミティブ単位で読み込む
	void processPrimitive(Mesh* mesh, int& indexStart, tinygltf::Primitive glPrimitive, tinygltf::Model glModel, std::shared_ptr<GltfModel> model);
	//gltfモデルのアニメーションを読み込む
	void loadAnimations(std::shared_ptr<GltfModel> model, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel);
	//gltfモデルのスケルトンを読み込む
	void loadSkin(std::shared_ptr<GltfModel> model, tinygltf::Model gltfModel);
	//自前のModelクラスにスケルトンを設定する
	void setSkin(GltfNode* node, std::shared_ptr<GltfModel> model);

	//テクスチャを読み込む
	void loadTextures(std::shared_ptr<GltfModel> model,const tinygltf::Model gltfModel);
	//マテリアルをメッシュに設定する
	void loadMaterial(std::shared_ptr<GltfModel> model,tinygltf::Model gltfModel);
	//埋め込まれたgltfモデルを取得する
	void loadgltfModel(int id, void** ptr, int& size);

	//luaスクリプトからSceneクラスを介して呼び出される。新しいモデルを求められたときのみ、解析処理をする
	std::shared_ptr<GltfModel> loadModel(std::string modelPath);

public:
	static FileManager* GetInstance()
	{
		if (!fileManager)
		{
			fileManager = new FileManager();
		}

		return fileManager;
	}

	static void FinishFileManger()
	{
		if (fileManager)
		{
			delete fileManager;
		}
	}

	~FileManager()
	{

		fileManager = nullptr;
	}

	//あとで読み取るgltfファイルとそれを持たせるModelクラスをリストに加える
	void addLoadModelList(std::string filePath, Model* model);

	//Modelクラスにgltfクラスを送る
	void setGltfModel();

	//luaスクリプトからSceneクラスを介して呼び出される。新しい画像が求められたときのみ、解析処理をする
	std::shared_ptr<ImageData> loadImage(std::string filePath);	
};