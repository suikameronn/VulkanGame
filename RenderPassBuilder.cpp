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
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withFormat(const VkFormat& format)
{
	description.format = format;

	return shared_from_this();
}

//マルチサンプリングのサンプル数を設定
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withMultiSamples(const VkSampleCountFlagBits& count)
{
	description.samples = count;

	return shared_from_this();
}

//カラーアタッチメントを使用する前の処理を指定
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withColorLoadOp(const VkAttachmentLoadOp& op)
{
	description.loadOp = op;

	return shared_from_this();
}

//カラーアタッチメントを使用した後の処理を指定
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withColorStoreOp(const VkAttachmentStoreOp& op)
{
	description.storeOp = op;

	return shared_from_this();
}

//ステンシルアタッチメントを使用する前の処理を指定
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withStencilLoadOp(const VkAttachmentLoadOp& op)
{
	description.stencilLoadOp = op;

	return shared_from_this();
}

//ステンシルアタッチメントを使用した後の処理を指定
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withStencilStoreOp(const VkAttachmentStoreOp& op)
{
	description.stencilStoreOp = op;

	return shared_from_this();
}

//サブパスに入る前のレイアウトを指定
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withInitialLayout(const VkImageLayout& layout)
{
	description.initialLayout = layout;

	return shared_from_this();
}

//サブパスに出た後のレイアウトを指定
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withFinalLayout(const VkImageLayout& layout)
{
	description.finalLayout = layout;

	return shared_from_this();
}

//ディスクリプションを追加する

//カラーアタッチメント
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addColorAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//初期化
	description = VkAttachmentDescription{};

	return shared_from_this();
}

//リゾルブカラーアタッチメントを追加
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addColorResolveAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorResolveReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//初期化
	description = VkAttachmentDescription{};

	return shared_from_this();
}

//デプスステンシルアタッチメントを追加
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addDepthStencilAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	property.depthReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//初期化
	description = VkAttachmentDescription{};

	return shared_from_this();
}

//VkSubpassDependencyの作成

	//一つ前のサブパスを指定する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withSrcSubpassIndex(const uint32_t& index)
{
	dependency.srcSubpass = index;

	return shared_from_this();
}

//一つ後のサブパスを指定する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withDstSubpassIndex(const uint32_t& index)
{
	dependency.dstSubpass = index;

	return shared_from_this();
}

//一つ前のサブパスがどのステージまで行くまで待つかを設定する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withSrcStageMask(const VkPipelineStageFlags& flag)
{
	dependency.srcStageMask = flag;

	return shared_from_this();
}

//一つ前のサブパスのメモリアクセスを待つのか設定する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withSrcAccessMask(const VkAccessFlags& mask)
{
	dependency.srcAccessMask = mask;

	return shared_from_this();
}

//このサブパスがどのステージで待つかを設定する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withDstStageMask(const VkPipelineStageFlags& flag)
{
	dependency.dstStageMask = flag;

	return shared_from_this();
}

//このサブパスのメモリアクセスで待つのか設定する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withDstAccessMask(const VkAccessFlags& mask)
{
	dependency.dstAccessMask = mask;

	return shared_from_this();
}

//このサブパスから遷移する範囲を設定する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withFlag(const VkDependencyFlags& flag)
{
	dependency.dependencyFlags = flag;

	return shared_from_this();
}

//サブパスの依存関係を追加する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addDependency()
{
	property.dependency.push_back(dependency);

	//構造体を初期化しておく
	dependency = VkSubpassDependency{};

	return shared_from_this();
}

//サブパスを追加する
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addSubpass()
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

	return shared_from_this();
}

//レンダーパスのプロパティを返す
RenderPassProperty RenderPassBuilder::Build()
{
	return property;
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