#include"RenderPassBuilder.h"

RenderPassBuilder::RenderPassBuilder(VkDevice& d)
{
	device = d;

	property.initProperty();
}

//プロパティを初期化する
void RenderPassBuilder::initProperty()
{
	description = VkAttachmentDescription{};
	subpassDescription = VkSubpassDescription{};
	dependency = VkSubpassDependency{};

	property.initProperty();
}

//VkAttachmentDescriptionの作成

//フォーマットの設定
RenderPassBuilder& RenderPassBuilder::withFormat(const VkFormat& format)
{
	description.format = format;

	return *this;
}

//マルチサンプリングのサンプル数を設定
RenderPassBuilder& RenderPassBuilder::withMultiSamples(const VkSampleCountFlagBits& count)
{
	description.samples = count;

	return *this;
}

//カラーアタッチメントを使用する前の処理を指定
RenderPassBuilder& RenderPassBuilder::withColorLoadOp(const VkAttachmentLoadOp& op)
{
	description.loadOp = op;

	return *this;
}

//カラーアタッチメントを使用した後の処理を指定
RenderPassBuilder& RenderPassBuilder::withColorStoreOp(const VkAttachmentStoreOp& op)
{
	description.storeOp = op;

	return *this;
}

//ステンシルアタッチメントを使用する前の処理を指定
RenderPassBuilder& RenderPassBuilder::withStencilLoadOp(const VkAttachmentLoadOp& op)
{
	description.stencilLoadOp = op;

	return *this;
}

//ステンシルアタッチメントを使用した後の処理を指定
RenderPassBuilder& RenderPassBuilder::withStencilStoreOp(const VkAttachmentStoreOp& op)
{
	description.stencilStoreOp = op;

	return *this;
}

//サブパスに入る前のレイアウトを指定
RenderPassBuilder& RenderPassBuilder::withInitialLayout(const VkImageLayout& layout)
{
	description.initialLayout = layout;

	return *this;
}

//サブパスに出た後のレイアウトを指定
RenderPassBuilder& RenderPassBuilder::withFinalLayout(const VkImageLayout& layout)
{
	description.finalLayout = layout;

	return *this;
}

//ディスクリプションを追加する

//カラーアタッチメント
RenderPassBuilder& RenderPassBuilder::addColorAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//初期化
	description = VkAttachmentDescription{};

	return *this;
}

//リゾルブカラーアタッチメントを追加
RenderPassBuilder& RenderPassBuilder::addColorResolveAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorResolveReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//初期化
	description = VkAttachmentDescription{};

	return *this;
}

//デプスステンシルアタッチメントを追加
RenderPassBuilder& RenderPassBuilder::addDepthStencilAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	property.depthReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//初期化
	description = VkAttachmentDescription{};

	return *this;
}

//VkSubpassDependencyの作成

	//一つ前のサブパスを指定する
RenderPassBuilder& RenderPassBuilder::withSrcSubpassIndex(const uint32_t& index)
{
	dependency.srcSubpass = index;

	return *this;
}

//一つ後のサブパスを指定する
RenderPassBuilder& RenderPassBuilder::withDstSubpassIndex(const uint32_t& index)
{
	dependency.dstSubpass = index;

	return *this;
}

//一つ前のサブパスがどのステージまで行くまで待つかを設定する
RenderPassBuilder& RenderPassBuilder::withSrcStageMask(const VkPipelineStageFlags& flag)
{
	dependency.srcStageMask = flag;

	return *this;
}

//一つ前のサブパスのメモリアクセスを待つのか設定する
RenderPassBuilder& RenderPassBuilder::withSrcAccessMask(const VkAccessFlags& mask)
{
	dependency.srcAccessMask = mask;

	return *this;
}

//このサブパスがどのステージで待つかを設定する
RenderPassBuilder& RenderPassBuilder::withDstStageMask(const VkPipelineStageFlags& flag)
{
	dependency.dstStageMask = flag;

	return *this;
}

//このサブパスのメモリアクセスで待つのか設定する
RenderPassBuilder& RenderPassBuilder::withDstAccessMask(const VkAccessFlags& mask)
{
	dependency.dstAccessMask = mask;

	return *this;
}

//このサブパスから遷移する範囲を設定する
RenderPassBuilder& RenderPassBuilder::withFlag(const VkDependencyFlags& flag)
{
	dependency.dependencyFlags = flag;

	return *this;
}

//サブパスの依存関係を追加する
RenderPassBuilder& RenderPassBuilder::addDependency()
{
	property.dependency.push_back(dependency);

	//構造体を初期化しておく
	dependency = VkSubpassDependency{};

	return *this;
}

//サブパスを追加する
RenderPassBuilder& RenderPassBuilder::addSubpass()
{
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	//オフセットと配列のサイズが同じな場合は、すでにその配列のアタッチメントはすべて
	//サブパスに組み込み済み
	if (property.colorCombinedOffset != property.colorReferences.size())
	{
		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(property.colorReferences.size()) - property.colorCombinedOffset;
		subpassDescription.pColorAttachments = &property.colorReferences[property.colorCombinedOffset];
	}

	if (property.colorResolveCombinedOffset != property.colorResolveReferences.size())
	{
		subpassDescription.pResolveAttachments = &property.colorResolveReferences[property.colorResolveCombinedOffset];
	}

	if (property.depthCombinedOffset != property.depthReferences.size())
	{
		subpassDescription.pDepthStencilAttachment = &property.depthReferences[property.colorCombinedOffset];
	}

	property.subpassDescription.push_back(subpassDescription);

	//構造体を初期化しておく
	subpassDescription = VkSubpassDescription{};

	return *this;
}

//レンダーパスのプロパティを返す
RenderPassProperty RenderPassBuilder::Build()
{
	RenderPassProperty p = property;

	property.initProperty();

	return p;
}

//VkRenderPassの作成する
void RenderPassBuilder::Create(const RenderPassProperty& p,VkRenderPass& pass)
{
	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = static_cast<uint32_t>(p.descriptions.size());
	info.pAttachments = p.descriptions.data();
	info.subpassCount = static_cast<uint32_t>(p.subpassDescription.size());
	info.pSubpasses = p.subpassDescription.data();
	info.dependencyCount = static_cast<uint32_t>(p.dependency.size());
	info.pDependencies = p.dependency.data();

	if (vkCreateRenderPass(device, &info, nullptr, &pass) != VK_SUCCESS) {
		throw std::runtime_error("RenderPassBuilder :: Create() :: レンダーパスの作成に失敗");
	}
}