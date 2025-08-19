#include"RenderPassBuilder.h"

RenderPassBuilder::RenderPassBuilder(VkDevice& d)
{
	device = d;

	property.initProperty();
}

//�v���p�e�B������������
void RenderPassBuilder::initProperty()
{
	description = VkAttachmentDescription{};
	subpassDescription = VkSubpassDescription{};
	dependency = VkSubpassDependency{};

	property.initProperty();
}

//VkAttachmentDescription�̍쐬

//�t�H�[�}�b�g�̐ݒ�
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withFormat(const VkFormat& format)
{
	description.format = format;

	return shared_from_this();
}

//�}���`�T���v�����O�̃T���v������ݒ�
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withMultiSamples(const VkSampleCountFlagBits& count)
{
	description.samples = count;

	return shared_from_this();
}

//�J���[�A�^�b�`�����g���g�p����O�̏������w��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withColorLoadOp(const VkAttachmentLoadOp& op)
{
	description.loadOp = op;

	return shared_from_this();
}

//�J���[�A�^�b�`�����g���g�p������̏������w��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withColorStoreOp(const VkAttachmentStoreOp& op)
{
	description.storeOp = op;

	return shared_from_this();
}

//�X�e���V���A�^�b�`�����g���g�p����O�̏������w��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withStencilLoadOp(const VkAttachmentLoadOp& op)
{
	description.stencilLoadOp = op;

	return shared_from_this();
}

//�X�e���V���A�^�b�`�����g���g�p������̏������w��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withStencilStoreOp(const VkAttachmentStoreOp& op)
{
	description.stencilStoreOp = op;

	return shared_from_this();
}

//�T�u�p�X�ɓ���O�̃��C�A�E�g���w��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withInitialLayout(const VkImageLayout& layout)
{
	description.initialLayout = layout;

	return shared_from_this();
}

//�T�u�p�X�ɏo����̃��C�A�E�g���w��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withFinalLayout(const VkImageLayout& layout)
{
	description.finalLayout = layout;

	return shared_from_this();
}

//�f�B�X�N���v�V������ǉ�����

//�J���[�A�^�b�`�����g
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addColorAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//������
	description = VkAttachmentDescription{};

	return shared_from_this();
}

//���]���u�J���[�A�^�b�`�����g��ǉ�
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addColorResolveAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorResolveReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//������
	description = VkAttachmentDescription{};

	return shared_from_this();
}

//�f�v�X�X�e���V���A�^�b�`�����g��ǉ�
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addDepthStencilAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	property.depthReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//������
	description = VkAttachmentDescription{};

	return shared_from_this();
}

//VkSubpassDependency�̍쐬

	//��O�̃T�u�p�X���w�肷��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withSrcSubpassIndex(const uint32_t& index)
{
	dependency.srcSubpass = index;

	return shared_from_this();
}

//���̃T�u�p�X���w�肷��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withDstSubpassIndex(const uint32_t& index)
{
	dependency.dstSubpass = index;

	return shared_from_this();
}

//��O�̃T�u�p�X���ǂ̃X�e�[�W�܂ōs���܂ő҂���ݒ肷��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withSrcStageMask(const VkPipelineStageFlags& flag)
{
	dependency.srcStageMask = flag;

	return shared_from_this();
}

//��O�̃T�u�p�X�̃������A�N�Z�X��҂̂��ݒ肷��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withSrcAccessMask(const VkAccessFlags& mask)
{
	dependency.srcAccessMask = mask;

	return shared_from_this();
}

//���̃T�u�p�X���ǂ̃X�e�[�W�ő҂���ݒ肷��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withDstStageMask(const VkPipelineStageFlags& flag)
{
	dependency.dstStageMask = flag;

	return shared_from_this();
}

//���̃T�u�p�X�̃������A�N�Z�X�ő҂̂��ݒ肷��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withDstAccessMask(const VkAccessFlags& mask)
{
	dependency.dstAccessMask = mask;

	return shared_from_this();
}

//���̃T�u�p�X����J�ڂ���͈͂�ݒ肷��
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::withFlag(const VkDependencyFlags& flag)
{
	dependency.dependencyFlags = flag;

	return shared_from_this();
}

//�T�u�p�X�̈ˑ��֌W��ǉ�����
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addDependency()
{
	property.dependency.push_back(dependency);

	//�\���̂����������Ă���
	dependency = VkSubpassDependency{};

	return shared_from_this();
}

//�T�u�p�X��ǉ�����
std::shared_ptr<RenderPassBuilder> RenderPassBuilder::addSubpass()
{
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	//�I�t�Z�b�g�Ɣz��̃T�C�Y�������ȏꍇ�́A���łɂ��̔z��̃A�^�b�`�����g�͂��ׂ�
	//�T�u�p�X�ɑg�ݍ��ݍς�
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

	//�\���̂����������Ă���
	subpassDescription = VkSubpassDescription{};

	return shared_from_this();
}

//�����_�[�p�X�̃v���p�e�B��Ԃ�
RenderPassProperty RenderPassBuilder::Build()
{
	return property;
}

//VkRenderPass�̍쐬����
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
		throw std::runtime_error("RenderPassBuilder :: Create() :: �����_�[�p�X�̍쐬�Ɏ��s");
	}
}