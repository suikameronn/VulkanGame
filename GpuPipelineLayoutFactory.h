#pragma once

#include"GpuPipelineLayoutBuilder.h"
#include"GpuDescriptorSetLayoutFactory.h"

#include"StructList.h"

enum class PipelineLayoutPattern
{
	PBR = 0,
	UI,
	CUBEMAP,
	CALC_SHADOWMAP,
	CALC_CUBEMAP,
	CALCIBL_DIFFUSE_SPECULAR,
	CALCIBL_BRDF,
	RAYCAST
};

struct PipelineLayout;

struct PipelineLayoutHash
{
	size_t operator()(const std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>,std::vector<VkPushConstantRange>> a) const
	{
		size_t hash = a.first.size();

		hash ^= a.second.size() + 0x9e3779b9 + (hash << 6) + (hash >> 2);

		for (const auto& layout : a.first) {
			hash ^= layout->hashKey + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}

		for (const auto& pushConstant : a.second)
		{
			hash ^= pushConstant.offset + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= pushConstant.size + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= static_cast<uint32_t>(pushConstant.stageFlags) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}

		return hash;
	}
};

inline bool operator==(const std::shared_ptr<DescriptorSetLayout>& lhs, const std::shared_ptr<DescriptorSetLayout>& rhs) 
{
	// 両方が nullptr なら等しい
	if (!lhs && !rhs) return true;

	// 片方だけが nullptr なら等しくない
	if (!lhs || !rhs) return false;
	
	// 両方が有効なら、指すオブジェクトの中身を比較する
	return *lhs == *rhs; // 上で定義した DescriptorSetLayout の operator== を呼び出す
}

inline bool operator==(const std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>, std::vector<VkPushConstantRange>> lhs
	, const std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>, std::vector<VkPushConstantRange>> rhs)
{
	for (int i = 0; i < lhs.first.size(); i++)
	{
		if (lhs.first[i] != rhs.first[i])
		{
			return false;
		}
	}

	for (int i = 0; i < lhs.second.size(); i++)
	{
		if (lhs.second[i].offset != rhs.second[i].offset ||
			lhs.second[i].size != rhs.second[i].size ||
			lhs.second[i].stageFlags != rhs.second[i].stageFlags)
		{
			return false;
		}
	}

	return true;
}

class GpuPipelineLayoutFactory : public std::enable_shared_from_this<GpuPipelineLayoutFactory>
{
private:

	//論理デバイス
	VkDevice device;

	//現在の破棄予定のリストのインデックス
	//描画処理が行われるごとに値が更新される
	uint32_t frameIndex;

	PipelineLayoutHash pipelineLayoutHash;

	//ビルダー
	std::shared_ptr<GpuPipelineLayoutBuilder> builder;

	//レイアウトを取得する
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;
	
	//既に作成したパイプラインレイアウトを格納する
	std::unordered_map<std::pair<std::vector<std::shared_ptr<DescriptorSetLayout>>, std::vector<VkPushConstantRange>>
		, std::weak_ptr<PipelineLayout>, PipelineLayoutHash> pipelineLayoutStorage;

	//破棄予定のリソースのリスト
	std::array<std::list<VkPipelineLayout>, 2> destructList;

	//既定のレイアウトから構造体を設定する
	void convertLayouts(PipelineLayoutPattern pattern
		, std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts, std::vector<VkPushConstantRange>& pushConstant);

	//ビルダーを使ってパイプラインレイアウトを作成する
	std::shared_ptr<PipelineLayout> createLayout(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts
		, std::vector<VkPushConstantRange>& pushConstants);

public:

	GpuPipelineLayoutFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutBuilder> b
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutF);

	~GpuPipelineLayoutFactory();

	//パイプラインレイアウトの作成
	std::shared_ptr<PipelineLayout> Create(PipelineLayoutPattern pattern);

	//パイプラインレイアウトの作成
	std::shared_ptr<PipelineLayout> Create(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts
		, std::vector<VkPushConstantRange>& pushConstants);

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

	std::shared_ptr<GpuPipelineLayoutFactory> factory;

	PipelineLayout(std::shared_ptr<GpuPipelineLayoutFactory> f)
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