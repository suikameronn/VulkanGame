#pragma once

#include<iostream>

#include"GltfModel.h"

#include"MaterialBuilder.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"
#include"DescriptorSetLayoutFactory.h"

class GltfModelFactory
{
private:

	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//マテリアルを読み込みする
	std::shared_ptr<MaterialBuilder> materialBuilder;
	//テクスチャを作成
	std::shared_ptr<TextureFactory> textureFactory;

	//ロードしたgltfモデルを保存するマップ
	std::unordered_map<uint32_t, std::shared_ptr<GltfModel>> modelStorage;

	//コライダー用のAABBの座標を求める
	glm::vec3 minPos;
	glm::vec3 maxPos;

	//モデル全体の頂点数とインデックス数
	int vertexNum;
	int indexNum;

	//ファイルパスからハッシュ値を計算する
	uint32_t hashFilePath(const std::string& filePath)
	{
		std::hash<std::string> hasher;
		return static_cast<uint32_t>(hasher(filePath));
	}

	//gltfモデルのノードを再帰的に読み込む
	void loadNode(std::shared_ptr<GltfModel> model, const int nodeIndex, const tinygltf::Model& gltfModel);
	void loadNode(size_t& offset, int parentIndex, std::shared_ptr<GltfModel> model
		, const int nodeIndex, const tinygltf::Model& gltfModel);

	//gltfモデルのメッシュを読み込む
	void loadMesh(const tinygltf::Node& gltfNode, const tinygltf::Model& gltfModel
		, Mesh& mesh, std::shared_ptr<GltfModel> model, int meshIndex);

	//プリミティブの読み取り
	void loadPrimitive(Mesh& mesh, int& indexStart
		, tinygltf::Primitive glPrimitive, tinygltf::Model glModel, std::shared_ptr<GltfModel> model);

	//アニメーションを読み込む
	void loadAnimations(std::shared_ptr<GltfModel> model, const tinygltf::Model& gltfModel);

	//スキンを読み込む
	void loadSkin(std::shared_ptr<GltfModel> model, tinygltf::Model gltfModel);

	//スキンを設定する
	void setSkin(std::shared_ptr<GltfModel> model);

	//マテリアルを読み込む
	void loadMaterial(std::shared_ptr<GltfModel> model, tinygltf::Model& gltfModel);

public:

	GltfModelFactory(std::shared_ptr<MaterialBuilder> builder
		, std::shared_ptr<TextureFactory> texture, std::shared_ptr<GpuBufferFactory> bf
		, std::shared_ptr<DescriptorSetLayoutFactory> layout, std::shared_ptr<DescriptorSetFactory> desc)
	{
		materialBuilder = builder;

		textureFactory = texture;

		bufferFactory = bf;

		layoutFactory = layout;

		descriptorSetFactory = desc;
	}

	~GltfModelFactory()
	{
#ifdef _DEBUG
		std::cout << "GltfModelFactory :: デストラクタ" << std::endl;
#endif
	}
	
	uint32_t Load(const std::string& filePath);

	std::shared_ptr<GltfModel> GetModel(uint32_t hash)
	{
		if (modelStorage.find(hash) != modelStorage.end())
		{
			return modelStorage[hash];
		}

		return nullptr;
	}

};