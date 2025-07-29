#pragma once

#include<iostream>
#include<vector>
#include<memory>
#include<unordered_map>

#include"DescriptorSetLayoutBuilder.h"

#define MAX_TEXTURE_COUNT 5

struct DescriptorSetLayout;
struct LayoutHash;

struct LayoutHash
{
	size_t genHash(const VkDescriptorSetLayoutBinding& a) const
	{
		size_t h1 = std::hash<uint32_t>{}(a.binding);
		size_t h2 = std::hash<VkDescriptorType>{}(a.descriptorType);
		size_t h3 = std::hash<uint32_t>{}(a.descriptorCount);
		size_t h4 = std::hash<VkShaderStageFlags>{}(a.stageFlags);
		size_t h5 = std::hash<const VkSampler*>{}(a.pImmutableSamplers);

		size_t seed = 0;

		seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h5 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

		return seed;
	}

	size_t genHash(const std::vector<VkDescriptorSetLayoutBinding>& a) const
	{
		size_t hash = a.size();

		for (const auto& binding : a) {
			hash ^= genHash(binding) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}
		return hash;
	}

	size_t operator()(const std::vector<VkDescriptorSetLayoutBinding>& a) const
	{
		size_t hash = a.size();

		for (const auto& binding : a) {
			hash ^= genHash(binding) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}
		return hash;
	}
};

inline bool operator==(const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs) {
	return lhs.binding == rhs.binding &&
		lhs.descriptorType == rhs.descriptorType &&
		lhs.descriptorCount == rhs.descriptorCount &&
		lhs.stageFlags == rhs.stageFlags &&
		lhs.pImmutableSamplers == rhs.pImmutableSamplers;
}

enum class LayoutPattern
{
	MODELANIMMAT = 0,//モデル行列とアニメーション行列
	VIEWPROJMAT,//ビュー行列とプロジェクション行列
	MATERIAL,//各種テクスチャを含む
	LIGHT,//ライトの色
	IBL,//IBLのディフーズ、スペキュラー、BRDFを含む
	RAYCAST,//二つのストレージバッファ
	UI,//UI用の変換行列とテクスチャ
	SINGLE_TEX_FLAG,//単一のテクスチャ(フラグメントシェーダ用)
	SINGLE_UNIFORM_VERT,//単一のユニフォームバッファ(頂点シェーダ用)
	SINGLE_UNIFORM_FRAG//単一のユニフォームバッファ(フラグメントシェーダ用)
};

class DescriptorSetLayoutFactory : public std::enable_shared_from_this<DescriptorSetLayoutFactory>
{
private:

	LayoutHash layoutHash;

	//バインディングをキーに、ある組み合わせのレイアウトを一つのみ保存する
	std::unordered_map<std::vector<VkDescriptorSetLayoutBinding>,
		std::weak_ptr<DescriptorSetLayout>,LayoutHash> layoutStorage;

	//論理デバイス
	VkDevice device;

	//現在の破棄予定のリストのインデックス
	//描画処理が行われるごとに値が更新される
	uint32_t frameIndex;

	//ビルダー
	std::shared_ptr<DescriptorSetLayoutBuilder> builder;

	//破棄予定のリソースのリスト
	std::array<std::list<VkDescriptorSetLayout>, 2> destructList;

	//レイアウトの作成
	std::shared_ptr<DescriptorSetLayout> createLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	//既定のレイアウトからビンディングを返す
	void convertBinding(const LayoutPattern& pattern, std::vector<VkDescriptorSetLayoutBinding>& bindings);

public:

	DescriptorSetLayoutFactory(VkDevice& d, std::shared_ptr<DescriptorSetLayoutBuilder> b);

	~DescriptorSetLayoutFactory();

	std::shared_ptr<DescriptorSetLayoutBuilder> getBuilder()
	{
		return builder;
	}

	//レイアウトの作成
	std::shared_ptr<DescriptorSetLayout> Create(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	
	//既定のプロパティから選択
	std::shared_ptr<DescriptorSetLayout> Create(const LayoutPattern& pattern);

	//遅延破棄リストにリソースを追加する
	void addDefferedDestruct(VkDescriptorSetLayout& layout);

	//リソースを破棄する
	void resourceDestruct();
};

struct DescriptorSetLayout
{
	VkDescriptorSetLayout layout;
	std::shared_ptr<DescriptorSetLayoutFactory> factory;

	//unordered_mapに登録するときに使うハッシュ値
	size_t hashKey;

	DescriptorSetLayout(std::shared_ptr<DescriptorSetLayoutFactory> f)
	{
		layout = nullptr;
		factory = f;
	}

	~DescriptorSetLayout()
	{
		//破棄予定リストにlayoutを追加する
		factory->addDefferedDestruct(layout);
	}

	bool operator==(const DescriptorSetLayout& other) const
	{
		if (hashKey == other.hashKey)
		{
			return true;
		}

		return false;
	}
};