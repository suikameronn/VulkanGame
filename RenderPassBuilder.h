#pragma once

#include<algorithm>

#include"VulkanCore.h"

inline bool operator==(const VkAttachmentDescription& lhs, const VkAttachmentDescription& rhs) {
	return lhs.flags == rhs.flags &&
		lhs.format == rhs.format &&
		lhs.samples == rhs.samples &&
		lhs.loadOp == rhs.loadOp &&
		lhs.storeOp == rhs.storeOp &&
		lhs.stencilLoadOp == rhs.stencilLoadOp &&
		lhs.stencilStoreOp == rhs.stencilStoreOp &&
		lhs.initialLayout == rhs.initialLayout &&
		lhs.finalLayout == rhs.finalLayout;
}

inline bool operator==(const VkAttachmentReference& lhs, const VkAttachmentReference& rhs) {
	return lhs.attachment == rhs.attachment &&
		lhs.layout == rhs.layout;
}

inline bool operator==(const VkSubpassDependency& lhs, const VkSubpassDependency& rhs) {
	return lhs.srcSubpass == rhs.srcSubpass &&
		lhs.dstSubpass == rhs.dstSubpass &&
		lhs.srcStageMask == rhs.srcStageMask &&
		lhs.dstStageMask == rhs.dstStageMask &&
		lhs.srcAccessMask == rhs.srcAccessMask &&
		lhs.dstAccessMask == rhs.dstAccessMask &&
		lhs.dependencyFlags == rhs.dependencyFlags;
}

inline bool operator==(const VkSubpassDescription& lhs, const VkSubpassDescription& rhs) {
	return lhs.flags == rhs.flags &&
		lhs.pipelineBindPoint == rhs.pipelineBindPoint &&
		lhs.inputAttachmentCount == rhs.inputAttachmentCount &&
		lhs.colorAttachmentCount == rhs.colorAttachmentCount &&
		lhs.pResolveAttachments == rhs.pResolveAttachments &&
		lhs.pDepthStencilAttachment == rhs.pDepthStencilAttachment &&
		lhs.preserveAttachmentCount == rhs.preserveAttachmentCount;
}

struct RenderPassProperty
{
	std::vector<VkAttachmentDescription> descriptions;

	std::vector<VkAttachmentReference> references;

	//すでにサブパスに含めたアタッチメントを除くためのもの
	uint32_t colorCombinedOffset;
	std::vector<VkAttachmentReference> colorReferences;
	//すでにサブパスに含めたアタッチメントを除くためのもの
	uint32_t depthCombinedOffset;
	std::vector<VkAttachmentReference> depthReferences;
	//すでにサブパスに含めたアタッチメントを除くためのもの
	uint32_t colorResolveCombinedOffset;
	std::vector<VkAttachmentReference> colorResolveReferences;

	std::vector<VkSubpassDependency> dependency;


	std::vector<VkSubpassDescription> subpassDescription;

	RenderPassProperty()
	{
		initProperty();
	}

	void initProperty()
	{
		colorCombinedOffset = 0;
		depthCombinedOffset = 0;
		colorResolveCombinedOffset = 0;

		descriptions.clear();
		references.clear();
		colorReferences.clear();
		depthReferences.clear();
		colorResolveReferences.clear();
		dependency.clear();
		subpassDescription.clear();
	}

	bool operator==(const RenderPassProperty& other) const
	{
		if(descriptions.size() != other.descriptions.size() ||
			references.size() != other.references.size() ||
			colorReferences.size() != other.colorReferences.size() ||
			depthReferences.size() != other.depthReferences.size() ||
			colorResolveReferences.size() != other.colorResolveReferences.size() ||
			dependency.size() != other.dependency.size() ||
			subpassDescription.size() != other.subpassDescription.size())
		{
			return false;
		}

		for(int i = 0; i < descriptions.size(); i++)
		{
			if (!(descriptions[i] == other.descriptions[i]))
			{
				return false;
			}
		}

		for(int i = 0; i < references.size(); i++)
		{
			if (!(references[i] == other.references[i]))
			{
				return false;
			}
		}

		for(int i = 0; i < colorReferences.size(); i++)
		{
			if (!(colorReferences[i] == other.colorReferences[i]))
			{
				return false;
			}
		}

		for(int i = 0; i < depthReferences.size(); i++)
		{
			if (!(depthReferences[i] == other.depthReferences[i]))
			{
				return false;
			}
		}

		for(int i = 0; i < colorResolveReferences.size(); i++)
		{
			if (!(colorResolveReferences[i] == other.colorResolveReferences[i]))
			{
				return false;
			}
		}

		for(int i = 0; i < dependency.size(); i++)
		{
			if (!(dependency[i] == other.dependency[i]))
			{
				return false;
			}
		}

		for(int i = 0; i < subpassDescription.size(); i++)
		{
			if (!(subpassDescription[i] == other.subpassDescription[i]))
			{
				return false;
			}
		}

		return true;
	}
};

class RenderPassBuilder : public std::enable_shared_from_this<RenderPassBuilder>
{
private:

	//論理デバイス
	VkDevice device;

	RenderPassProperty property;

	VkAttachmentDescription description;

	VkSubpassDependency dependency;

	VkSubpassDescription subpassDescription;

public:

	RenderPassBuilder(VkDevice& d);

	~RenderPassBuilder()
	{
#ifdef _DEBUG
		std::cout << "RenderPassBuilder :: デストラクタ" << std::endl;
#endif
	}

	//プロパティを初期化する
	void initProperty();

	//VkAttachmentDescriptionの作成

	//フォーマットの設定
	std::shared_ptr<RenderPassBuilder> withFormat(const VkFormat& format);
	//マルチサンプリングのサンプル数を設定
	std::shared_ptr<RenderPassBuilder> withMultiSamples(const VkSampleCountFlagBits& count);
	//カラーアタッチメントを使用する前の処理を指定
	std::shared_ptr<RenderPassBuilder> withColorLoadOp(const VkAttachmentLoadOp& op);
	//カラーアタッチメントを使用した後の処理を指定
	std::shared_ptr<RenderPassBuilder> withColorStoreOp(const VkAttachmentStoreOp& op);
	//ステンシルアタッチメントを使用する前の処理を指定
	std::shared_ptr<RenderPassBuilder> withStencilLoadOp(const VkAttachmentLoadOp& op);
	//ステンシルアタッチメントを使用した後の処理を指定
	std::shared_ptr<RenderPassBuilder> withStencilStoreOp(const VkAttachmentStoreOp& op);
	//サブパスに入る前のレイアウトを指定
	std::shared_ptr<RenderPassBuilder> withInitialLayout(const VkImageLayout& layout);
	//サブパスに出た後のレイアウトを指定
	std::shared_ptr<RenderPassBuilder> withFinalLayout(const VkImageLayout& layout);
	//ディスクリプションを追加する
	std::shared_ptr<RenderPassBuilder> addColorAttachment();
	std::shared_ptr<RenderPassBuilder> addColorResolveAttachment();
	std::shared_ptr<RenderPassBuilder> addDepthStencilAttachment();

	//VkSubpassDependencyの作成

	//一つ前のサブパスを指定する
	std::shared_ptr<RenderPassBuilder> withSrcSubpassIndex(const uint32_t& index);
	//一つ後のサブパスを指定する
	std::shared_ptr<RenderPassBuilder> withDstSubpassIndex(const uint32_t& index);
	//一つ前のサブパスがどのステージまで行くまで待つかを設定する
	std::shared_ptr<RenderPassBuilder> withSrcStageMask(const VkPipelineStageFlags& flag);
	//一つ前のサブパスのメモリアクセスを待つのか設定する
	std::shared_ptr<RenderPassBuilder> withSrcAccessMask(const VkAccessFlags& mask);
	//このサブパスがどのステージで待つかを設定する
	std::shared_ptr<RenderPassBuilder> withDstStageMask(const VkPipelineStageFlags& flag);
	//このサブパスのメモリアクセスで待つのか設定する
	std::shared_ptr<RenderPassBuilder> withDstAccessMask(const VkAccessFlags& mask);
	//このサブパスから遷移する範囲を設定する
	std::shared_ptr<RenderPassBuilder> withFlag(const VkDependencyFlags& flag);
	//サブパスの依存関係を追加する
	std::shared_ptr<RenderPassBuilder> addDependency();
	//サブパスを追加する
	std::shared_ptr<RenderPassBuilder> addSubpass();

	//プロパティからVkRenderPassCreateInfoを返す
	RenderPassProperty Build();

	//VkRenderPassを作成する
	void Create(const RenderPassProperty& p, VkRenderPass& pass);
};