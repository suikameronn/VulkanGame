#include"GpuRenderPassBuilder.h"

GpuRenderPassBuilder::GpuRenderPassBuilder(VkDevice& d)
{
	device = d;

	property.initProperty();
}

//�v���p�e�B������������
void GpuRenderPassBuilder::initProperty()
{
	property.initProperty();
}

//VkAttachmentDescription�̍쐬

//�t�H�[�}�b�g�̐ݒ�
GpuRenderPassBuilder GpuRenderPassBuilder::withFormat(const VkFormat& format)
{
	description.format = format;

	return *this;
}

//�}���`�T���v�����O�̃T���v������ݒ�
GpuRenderPassBuilder GpuRenderPassBuilder::withMultiSamples(const VkSampleCountFlagBits& count)
{
	description.samples = count;

	return *this;
}

//�J���[�A�^�b�`�����g���g�p����O�̏������w��
GpuRenderPassBuilder GpuRenderPassBuilder::withColorLoadOp(const VkAttachmentLoadOp& op)
{
	description.loadOp = op;

	return *this;
}

//�J���[�A�^�b�`�����g���g�p������̏������w��
GpuRenderPassBuilder GpuRenderPassBuilder::withColorStoreOp(const VkAttachmentStoreOp& op)
{
	description.storeOp = op;

	return *this;
}

//�X�e���V���A�^�b�`�����g���g�p����O�̏������w��
GpuRenderPassBuilder GpuRenderPassBuilder::withStencilLoadOp(const VkAttachmentLoadOp& op)
{
	description.stencilLoadOp = op;

	return *this;
}

//�X�e���V���A�^�b�`�����g���g�p������̏������w��
GpuRenderPassBuilder GpuRenderPassBuilder::withStencilStoreOp(const VkAttachmentStoreOp& op)
{
	description.stencilStoreOp = op;

	return *this;
}

//�T�u�p�X�ɓ���O�̃��C�A�E�g���w��
GpuRenderPassBuilder GpuRenderPassBuilder::withInitialLayout(const VkImageLayout& layout)
{
	description.initialLayout = layout;

	return *this;
}

//�T�u�p�X�ɏo����̃��C�A�E�g���w��
GpuRenderPassBuilder GpuRenderPassBuilder::withFinalLayout(const VkImageLayout& layout)
{
	description.finalLayout = layout;

	return *this;
}

//�f�B�X�N���v�V������ǉ�����

//�J���[�A�^�b�`�����g
GpuRenderPassBuilder GpuRenderPassBuilder::addColorAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//������
	description = VkAttachmentDescription{};

	return *this;
}

//���]���u�J���[�A�^�b�`�����g��ǉ�
GpuRenderPassBuilder GpuRenderPassBuilder::addColorResolveAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	property.colorResolveReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//������
	description = VkAttachmentDescription{};

	return *this;
}

//�f�v�X�X�e���V���A�^�b�`�����g��ǉ�
GpuRenderPassBuilder GpuRenderPassBuilder::addDepthStencilAttachment()
{
	VkAttachmentReference attachment{};
	attachment.attachment = static_cast<uint32_t>(property.descriptions.size());
	attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	property.depthReferences.push_back(attachment);
	property.descriptions.push_back(description);

	//������
	description = VkAttachmentDescription{};

	return *this;
}

//VkSubpassDependency�̍쐬

	//��O�̃T�u�p�X���w�肷��
GpuRenderPassBuilder GpuRenderPassBuilder::withSrcSubpassIndex(const uint32_t& index)
{
	dependency.srcSubpass = index;

	return *this;
}

//���̃T�u�p�X���w�肷��
GpuRenderPassBuilder GpuRenderPassBuilder::withDstSubpassIndex(const uint32_t& index)
{
	dependency.dstSubpass = index;

	return *this;
}

//��O�̃T�u�p�X���ǂ̃X�e�[�W�܂ōs���܂ő҂���ݒ肷��
GpuRenderPassBuilder GpuRenderPassBuilder::withSrcStageMask(const VkPipelineStageFlags& flag)
{
	dependency.srcStageMask = flag;

	return *this;
}

//��O�̃T�u�p�X�̃������A�N�Z�X��҂̂��ݒ肷��
GpuRenderPassBuilder GpuRenderPassBuilder::withSrcAccessMask(const VkAccessFlags& mask)
{
	dependency.srcAccessMask = mask;

	return *this;
}

//���̃T�u�p�X���ǂ̃X�e�[�W�ő҂���ݒ肷��
GpuRenderPassBuilder GpuRenderPassBuilder::withDstStageMask(const VkPipelineStageFlags& flag)
{
	dependency.dstStageMask = flag;

	return *this;
}

//���̃T�u�p�X�̃������A�N�Z�X�ő҂̂��ݒ肷��
GpuRenderPassBuilder GpuRenderPassBuilder::withDstAccessMask(const VkAccessFlags& mask)
{
	dependency.dstAccessMask = mask;

	return *this;
}

//���̃T�u�p�X����J�ڂ���͈͂�ݒ肷��
GpuRenderPassBuilder GpuRenderPassBuilder::withFlag(const VkDependencyFlags& flag)
{
	dependency.dependencyFlags = flag;

	return *this;
}

//�T�u�p�X�̈ˑ��֌W��ǉ�����
GpuRenderPassBuilder GpuRenderPassBuilder::addDependency()
{
	property.dependency.push_back(dependency);

	//�\���̂����������Ă���
	dependency = VkSubpassDependency{};

	return *this;
}

//�T�u�p�X��ǉ�����
GpuRenderPassBuilder GpuRenderPassBuilder::addSubpass()
{
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	//�I�t�Z�b�g�Ɣz��̃T�C�Y�������ȏꍇ�́A���łɂ��̔z��̃A�^�b�`�����g�͂��ׂ�
	//�T�u�p�X�ɑg�ݍ��ݍς�
	if (property.colorCombinedOffset != property.colorReferences.size())
	{
		subpassDescription.colorAttachmentCount = property.colorReferences.size() - property.colorCombinedOffset;
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

	return *this;
}

//�����_�[�p�X�̃v���p�e�B��Ԃ�
RenderPassProperty GpuRenderPassBuilder::Build()
{
	RenderPassProperty p = property;

	property.initProperty();

	return p;
}

//VkRenderPass�̍쐬����
void GpuRenderPassBuilder::Create(const RenderPassProperty& p,VkRenderPass& pass)
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
		throw std::runtime_error("GpuRenderPassBuilder :: Create() :: �����_�[�p�X�̍쐬�Ɏ��s");
	}
}