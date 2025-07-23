#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include<iostream>
#include<vector>
#include<algorithm>

#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

#include"Object.h"

//シェーダ上のテクスチャの参照用番号
//マテリアルがそのテクスチャを持たない場合は、-1が記録される
struct TexCoordSets 
{
	int baseColor;
	int metallicRoughness;
	int specularGlossiness;
	int normal;
	int occlusion;
	int emissive;

	void init()
	{
		baseColor = -1;
		metallicRoughness = -1;
		specularGlossiness = -1;
		normal = -1;
		occlusion = -1;
		emissive = -1;
	};
};

struct MaterialProperty
{
	TexCoordSets texCoordSet;

	std::shared_ptr<Texture> baseColorTexture;
	std::shared_ptr<Texture> metallicRoughnessTexture;
	std::shared_ptr<Texture> normalTexture;
	std::shared_ptr<Texture> occlusionTexture;
	std::shared_ptr<Texture> emissiveTexture;

	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;

	float metallicFactor;
	float roughnessFactor;
	float alphaMaskCutoff;
	float emissiveStrength;

	void initProperty()
	{
		texCoordSet.init();

		//透明度の最大値の設定
		alphaMaskCutoff = 1.0f;
		//メタリックの初期値
		metallicFactor = 1.0f;
		//粗さの初期値
		roughnessFactor = 1.0f;
		//自己放射の強さ
		emissiveStrength = 1.0f;
		//基本色の設定
		baseColorFactor = glm::vec4(1.0f);
		//発光
		emissiveFactor = glm::vec4(0.0f);

		baseColorTexture = nullptr;
		metallicRoughnessTexture = nullptr;
		normalTexture = nullptr;
		occlusionTexture = nullptr;
		emissiveTexture = nullptr;
	}

	void Build(std::shared_ptr<Texture> emptyTexture)
	{
		if (!baseColorTexture)
		{
			baseColorTexture = emptyTexture;
		}

		if (!metallicRoughnessTexture)
		{
			metallicRoughnessTexture = emptyTexture;
		}

		if (!normalTexture)
		{
			normalTexture = emptyTexture;
		}

		if (!occlusionTexture)
		{
			occlusionTexture = emptyTexture;
		}

		if (!emissiveTexture)
		{
			emissiveTexture = emptyTexture;
		}
	}
};

//シェーダ上でアクセスるためのマテリアル
//テクスチャに対する係数がメイン
struct ShaderMaterial
{
	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;
	glm::vec4 diffuseFactor;
	glm::vec4 specularFactor;
	int colorTextureSet;
	int PhysicalDescriptorTextureSet;
	int normalTextureSet;
	int occlusionTextureSet;
	int emissiveTextureSet;
	float metallicFactor;
	float roughnessFactor;
	float alphaMask;
	float alphaMaskCutoff;
	float emissiveStrength;

	void init(const MaterialProperty& prop)
	{
		baseColorFactor = prop.baseColorFactor;
		emissiveFactor = prop.emissiveFactor;
		diffuseFactor = glm::vec4(1.0f);
		specularFactor = glm::vec4(1.0f);
		
		colorTextureSet = prop.texCoordSet.baseColor;
		PhysicalDescriptorTextureSet = prop.texCoordSet.metallicRoughness;
		normalTextureSet = prop.texCoordSet.normal;
		occlusionTextureSet = prop.texCoordSet.occlusion;
		emissiveTextureSet = prop.texCoordSet.emissive;
		metallicFactor = prop.metallicFactor;
		roughnessFactor = prop.roughnessFactor;
		alphaMaskCutoff = prop.alphaMaskCutoff;
		alphaMask = 1.0f - alphaMaskCutoff; //透明度の最大値
		emissiveStrength = prop.emissiveStrength;
	}
};

//gltfモデルのマテリアル
class Material
{
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//gpu上のテクスチャに対する係数用のバッファ
	std::shared_ptr<GpuBuffer> shaderMaterialBuffer;

	//gpu上のマテリアルに関するデータを紐づけるもの
	//レンダリング時に使用
	std::shared_ptr<DescriptorSet> descriptorSet;

	//シェーダー上のマテリアルデータを記録するバッファを作成
	void createBuffer();
	//ディスクリプタセットを作成
	void createDescriptorSet();

public:

	Material(const MaterialProperty& prop)
	{
		shaderMaterial.init(prop);
		texCoordSets = prop.texCoordSet;

		//gpuリソースの作成
		createBuffer();
		createDescriptorSet();

		//更新した構造体の値をgpu上のバッファにコピー
		bufferFactory->copyMemory(
			sizeof(ShaderMaterial),
			&shaderMaterial,
			shaderMaterialBuffer,
			false
		);
	}

	enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
	AlphaMode alphaMode = ALPHAMODE_OPAQUE;

	//gltfモデル内のマテリアルの番号
	//gltfモデルのメッシュが持つマテリアルを参照する際に使用
	int index = 0;

	//テクスチャが従うuvの種類を記録,-1の場合はそのテクスチャを持たないことを示す
	TexCoordSets texCoordSets;

	//テクスチャに対する係数
	ShaderMaterial shaderMaterial;

	std::shared_ptr<DescriptorSet> getDescriptorSet() const
	{
		return descriptorSet;
	}

	std::shared_ptr<GpuBuffer> getShaderMaterialBuffer() const
	{
		return shaderMaterialBuffer;
	}	
};