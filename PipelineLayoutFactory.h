#pragma once

#include"PipelineLayoutBuilder.h"
#include"DescriptorSetLayoutFactory.h"

#include"StructList.h"

enum class PipelineLayoutPattern
{
	PBR = 0,
	UI,
	CUBEMAP,
	CALC_SHADOWMAP,
	CALC_CUBEMAP,
	CALC_IBL_DIFFUSE,
	CALC_IBL_SPECULAR,
	CALC_IBL_BRDF,
	RAYCAST
};

struct PipelineLayout;

struct PipelineLayoutHash
{
	const uint64_t FNV_PRIME = 1099511628211LLU; // 2^40 + 2^8 + 0xB3 = 0x100000001b3
	const uint64_t FNV_OFFSET_BIAS = 14695981039346656037U; // 0xcbf29ce484222325

	void getHash(const uint32_t& u, size_t& hash) const
	{
		uint64_t u64 = u;

		hash ^= u64;
		hash *= FNV_PRIME;
	}

	void getHash(const float f, size_t& hash) const
	{
		uint32_t u;
		std::memcpy(&u, &f, sizeof(float));

		getHash(u, hash);
	}

	size_t operator()(const PipelineLayoutProperty& p) const
	{
		size_t hash = FNV_OFFSET_BIAS;

		getHash(static_cast<uint32_t>(p.layoutArray.size()), hash);
		for (int i = 0; i < p.layoutArray.size(); i++)
		{
			getHash(static_cast<uint32_t>(p.layoutArray[i]->hashKey), hash);
		}

		getHash(static_cast<uint32_t>(p.layoutArray.size()), hash);
		for (int i = 0; i < p.pushconstantArray.size(); i++)
		{
			getHash(static_cast<uint32_t>(p.pushconstantArray[i].size), hash);
			getHash(static_cast<uint32_t>(p.pushconstantArray[i].stageFlags), hash);
		}

		return hash;
	};
};

class PipelineLayoutFactory : public std::enable_shared_from_this<PipelineLayoutFactory>
{
private:

	//論理デバイス
	VkDevice device;

	//現在の破棄予定のリストのインデックス
	//描画処理が行われるごとに値が更新される
	uint32_t frameIndex;

	PipelineLayoutHash pipelineLayoutHash;

	//ビルダー
	std::shared_ptr<PipelineLayoutBuilder> builder;

	//レイアウトを取得する
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	
	//既に作成したパイプラインレイアウトを格納する
	std::unordered_map<PipelineLayoutProperty, std::weak_ptr<PipelineLayout>, PipelineLayoutHash> pipelineLayoutStorage;

	//破棄予定のリソースのリスト
	std::array<std::list<VkPipelineLayout>, 2> destructList;

	//既定のレイアウトから構造体を設定する
	PipelineLayoutProperty convertLayouts(const PipelineLayoutPattern& pattern);

	//ビルダーを使ってパイプラインレイアウトを作成する
	std::shared_ptr<PipelineLayout> createLayout(const PipelineLayoutProperty& property);

public:

	PipelineLayoutFactory(VkDevice& d, std::shared_ptr<PipelineLayoutBuilder> b
		, std::shared_ptr<DescriptorSetLayoutFactory> layoutF);

	~PipelineLayoutFactory();

	//パイプラインレイアウトの作成
	std::shared_ptr<PipelineLayout> Create(const PipelineLayoutPattern& pattern);

	//パイプラインレイアウトの作成
	std::shared_ptr<PipelineLayout> Create(const PipelineLayoutProperty& property);

	//遅延破棄リストにリソースを追加する
	void addDefferedDestruct(VkPipelineLayout& pLayout);

	//リソースを破棄する
	void resourceDestruct();

};

struct PipelineLayout
{
	//ハッシュ値
	size_t hashKey;

	VkPipelineLayout pLayout;

	//パイプラインレイアウトで使ったレイアウト
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts;

	//VkPushconstantの配列
	std::vector<VkPushConstantRange> pushconstants;

	std::shared_ptr<PipelineLayoutFactory> factory;

	PipelineLayout(std::shared_ptr<PipelineLayoutFactory> f)
	{
		pLayout = nullptr;
		hashKey = 0;

		factory = f;
	}

	~PipelineLayout()
	{
		//破棄予定リストにリソースを加える
		factory->addDefferedDestruct(pLayout);
	}
};