#pragma once

#include<algorithm>

#include"VulkanCore.h"

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
		descriptions.clear();
		references.clear();
		colorReferences.clear();
		depthReferences.clear();
		colorResolveReferences.clear();
		dependency.clear();
		subpassDescription.clear();
	}
};

class GpuRenderPassBuilder
{
private:

	//論理デバイス
	VkDevice device;

	RenderPassProperty property;

	VkAttachmentDescription description;

	VkSubpassDependency dependency;

	VkSubpassDescription subpassDescription;

public:

	GpuRenderPassBuilder(VkDevice& d);

	//プロパティを初期化する
	void initProperty();

	//VkAttachmentDescriptionの作成

	//フォーマットの設定
	GpuRenderPassBuilder withFormat(const VkFormat& format);
	//マルチサンプリングのサンプル数を設定
	GpuRenderPassBuilder withMultiSamples(const VkSampleCountFlagBits& count);
	//カラーアタッチメントを使用する前の処理を指定
	GpuRenderPassBuilder withColorLoadOp(const VkAttachmentLoadOp& op);
	//カラーアタッチメントを使用した後の処理を指定
	GpuRenderPassBuilder withColorStoreOp(const VkAttachmentStoreOp& op);
	//ステンシルアタッチメントを使用する前の処理を指定
	GpuRenderPassBuilder withStencilLoadOp(const VkAttachmentLoadOp& op);
	//ステンシルアタッチメントを使用した後の処理を指定
	GpuRenderPassBuilder withStencilStoreOp(const VkAttachmentStoreOp& op);
	//サブパスに入る前のレイアウトを指定
	GpuRenderPassBuilder withInitialLayout(const VkImageLayout& layout);
	//サブパスに出た後のレイアウトを指定
	GpuRenderPassBuilder withFinalLayout(const VkImageLayout& layout);
	//ディスクリプションを追加する
	GpuRenderPassBuilder addColorAttachment();
	GpuRenderPassBuilder addColorResolveAttachment();
	GpuRenderPassBuilder addDepthStencilAttachment();

	//VkSubpassDependencyの作成

	//一つ前のサブパスを指定する
	GpuRenderPassBuilder withSrcSubpassIndex(const uint32_t& index);
	//一つ後のサブパスを指定する
	GpuRenderPassBuilder withDstSubpassIndex(const uint32_t& index);
	//一つ前のサブパスがどのステージまで行くまで待つかを設定する
	GpuRenderPassBuilder withSrcStageMask(const VkPipelineStageFlags& flag);
	//一つ前のサブパスのメモリアクセスを待つのか設定する
	GpuRenderPassBuilder withSrcAccessMask(const VkAccessFlags& mask);
	//このサブパスがどのステージで待つかを設定する
	GpuRenderPassBuilder withDstStageMask(const VkPipelineStageFlags& flag);
	//このサブパスのメモリアクセスで待つのか設定する
	GpuRenderPassBuilder withDstAccessMask(const VkAccessFlags& mask);
	//このサブパスから遷移する範囲を設定する
	GpuRenderPassBuilder withFlag(const VkDependencyFlags& flag);
	//サブパスの依存関係を追加する
	GpuRenderPassBuilder addDependency();
	//サブパスを追加する
	GpuRenderPassBuilder addSubpass();

	//プロパティからVkRenderPassCreateInfoを返す
	RenderPassProperty Build();

	//VkRenderPassを作成する
	void Create(const RenderPassProperty& p, VkRenderPass& pass);
};