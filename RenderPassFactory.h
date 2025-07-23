#pragma once

#include<iostream>
#include<unordered_map>
#include<array>
#include<list>

#include"RenderPassBuilder.h"

struct RenderPass;

enum class RenderPassPattern
{
	PBR,
	UI,
	CALC_SHADOWMAP,
	CALC_CUBEMAP,
	CALC_IBL
};

struct RenderPassHash
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

	void getHash(const std::vector<VkAttachmentDescription>& descriptions, size_t& hash) const
	{
		for (const auto& description : descriptions)
		{
			getHash(static_cast<uint32_t>(description.format), hash);
			getHash(static_cast<uint32_t>(description.samples), hash);
			getHash(static_cast<uint32_t>(description.loadOp), hash);
			getHash(static_cast<uint32_t>(description.storeOp), hash);
			getHash(static_cast<uint32_t>(description.stencilLoadOp), hash);
			getHash(static_cast<uint32_t>(description.stencilStoreOp), hash);
			getHash(static_cast<uint32_t>(description.initialLayout), hash);
			getHash(static_cast<uint32_t>(description.finalLayout), hash);
		}
	}

	void getHash(const VkAttachmentReference& reference, size_t& hash) const
	{
		getHash(reference.attachment, hash);
		getHash(static_cast<uint32_t>(reference.layout), hash);
	}

	void getHash(const std::vector<VkAttachmentReference>& references, size_t& hash) const
	{
		for (const auto& ref : references)
		{
			getHash(ref, hash);
		}
	}

	void getHash(const std::vector<VkSubpassDependency>& dependencies, size_t& hash) const
	{
		for (const auto& dependency : dependencies)
		{
			getHash(static_cast<uint32_t>(dependency.srcSubpass), hash);
			getHash(static_cast<uint32_t>(dependency.srcStageMask), hash);
			getHash(static_cast<uint32_t>(dependency.srcAccessMask), hash);
			getHash(static_cast<uint32_t>(dependency.dstStageMask), hash);
			getHash(static_cast<uint32_t>(dependency.dstAccessMask), hash);
			getHash(static_cast<uint32_t>(dependency.dstSubpass), hash);
			getHash(static_cast<uint32_t>(dependency.dependencyFlags), hash);
		}
	}

	void getHash(const std::vector<VkSubpassDescription>& descriptions, size_t& hash) const
	{
		for (const auto& description : descriptions)
		{
			getHash(static_cast<uint32_t>(description.flags), hash);
			getHash(static_cast<uint32_t>(description.pipelineBindPoint), hash);
			getHash(static_cast<uint32_t>(description.inputAttachmentCount), hash);

			for (uint32_t i = 0; i < description.inputAttachmentCount; i++)
			{
				VkAttachmentReference ref = description.pInputAttachments[i];

				getHash(ref, hash);
			}

			getHash(static_cast<uint32_t>(description.colorAttachmentCount), hash);

			for (uint32_t i = 0; i < description.colorAttachmentCount; i++)
			{
				VkAttachmentReference ref = description.pColorAttachments[i];

				getHash(ref, hash);
			}
		}
	}

	size_t operator()(const RenderPassProperty& a) const
	{
		size_t hash = FNV_OFFSET_BIAS;

		getHash(a.colorCombinedOffset, hash);
		getHash(a.colorResolveCombinedOffset, hash);
		getHash(a.depthCombinedOffset, hash);

		getHash(a.descriptions, hash);

		getHash(a.colorReferences, hash);
		getHash(a.colorResolveReferences, hash);
		getHash(a.depthReferences, hash);

		getHash(a.descriptions, hash);
		
		getHash(a.references, hash);

		return hash;
	}
};

class RenderPassFactory : public std::enable_shared_from_this<RenderPassFactory>
{
private:

	RenderPassHash hash;

	//フレームインデックス
	uint32_t frameIndex;

	VkDevice device;

	std::shared_ptr<RenderPassBuilder> builder;

	//破棄予定リスト
	std::array<std::list<VkRenderPass>, 2> destructList;

	//作成したレンダーパスをここに記録する
	std::unordered_map<RenderPassProperty, std::weak_ptr<RenderPass>, RenderPassHash> renderPassStorage;

	RenderPassProperty convertPattern(const RenderPassPattern& pattern);

public:

	RenderPassFactory(VkDevice& d, std::shared_ptr<RenderPassBuilder> b);

	//レンダーパスを作成する
	std::shared_ptr<RenderPass> Create(const RenderPassProperty& property);
	//レンダーパスを作成する
	std::shared_ptr<RenderPass> Create(const RenderPassPattern& pattern);

	//遅延破棄リストにリソースを追加する
	void addDefferedDestruct(VkRenderPass& renderPass);

	//リソースを破棄する
	void resourceDestruct();
};

struct RenderPass
{
	size_t hashKey;

	VkRenderPass renderPass;

	std::shared_ptr<RenderPassFactory> factory;

	RenderPass(std::shared_ptr<RenderPassFactory> f)
	{
		hashKey = 0;

		renderPass = nullptr;

		factory = f;
	}

	~RenderPass()
	{
		factory->addDefferedDestruct(renderPass);
	}
};

inline bool operator==(const std::pair<RenderPassProperty, std::shared_ptr<RenderPass>> lhs
	, const std::pair<RenderPassProperty, std::shared_ptr<RenderPass>> rhs)
{
	if (lhs.first != rhs.first)
	{
		return false;
	}

	if (lhs.second->hashKey != rhs.second->hashKey)
	{
		return false;
	}

	return true;
}