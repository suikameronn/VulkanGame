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

	//���łɃT�u�p�X�Ɋ܂߂��A�^�b�`�����g���������߂̂���
	uint32_t colorCombinedOffset;
	std::vector<VkAttachmentReference> colorReferences;
	//���łɃT�u�p�X�Ɋ܂߂��A�^�b�`�����g���������߂̂���
	uint32_t depthCombinedOffset;
	std::vector<VkAttachmentReference> depthReferences;
	//���łɃT�u�p�X�Ɋ܂߂��A�^�b�`�����g���������߂̂���
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

class RenderPassBuilder
{
private:

	//�_���f�o�C�X
	VkDevice device;

	RenderPassProperty property;

	VkAttachmentDescription description;

	VkSubpassDependency dependency;

	VkSubpassDescription subpassDescription;

public:

	RenderPassBuilder(VkDevice& d);

	//�v���p�e�B������������
	void initProperty();

	//VkAttachmentDescription�̍쐬

	//�t�H�[�}�b�g�̐ݒ�
	RenderPassBuilder withFormat(const VkFormat& format);
	//�}���`�T���v�����O�̃T���v������ݒ�
	RenderPassBuilder withMultiSamples(const VkSampleCountFlagBits& count);
	//�J���[�A�^�b�`�����g���g�p����O�̏������w��
	RenderPassBuilder withColorLoadOp(const VkAttachmentLoadOp& op);
	//�J���[�A�^�b�`�����g���g�p������̏������w��
	RenderPassBuilder withColorStoreOp(const VkAttachmentStoreOp& op);
	//�X�e���V���A�^�b�`�����g���g�p����O�̏������w��
	RenderPassBuilder withStencilLoadOp(const VkAttachmentLoadOp& op);
	//�X�e���V���A�^�b�`�����g���g�p������̏������w��
	RenderPassBuilder withStencilStoreOp(const VkAttachmentStoreOp& op);
	//�T�u�p�X�ɓ���O�̃��C�A�E�g���w��
	RenderPassBuilder withInitialLayout(const VkImageLayout& layout);
	//�T�u�p�X�ɏo����̃��C�A�E�g���w��
	RenderPassBuilder withFinalLayout(const VkImageLayout& layout);
	//�f�B�X�N���v�V������ǉ�����
	RenderPassBuilder addColorAttachment();
	RenderPassBuilder addColorResolveAttachment();
	RenderPassBuilder addDepthStencilAttachment();

	//VkSubpassDependency�̍쐬

	//��O�̃T�u�p�X���w�肷��
	RenderPassBuilder withSrcSubpassIndex(const uint32_t& index);
	//���̃T�u�p�X���w�肷��
	RenderPassBuilder withDstSubpassIndex(const uint32_t& index);
	//��O�̃T�u�p�X���ǂ̃X�e�[�W�܂ōs���܂ő҂���ݒ肷��
	RenderPassBuilder withSrcStageMask(const VkPipelineStageFlags& flag);
	//��O�̃T�u�p�X�̃������A�N�Z�X��҂̂��ݒ肷��
	RenderPassBuilder withSrcAccessMask(const VkAccessFlags& mask);
	//���̃T�u�p�X���ǂ̃X�e�[�W�ő҂���ݒ肷��
	RenderPassBuilder withDstStageMask(const VkPipelineStageFlags& flag);
	//���̃T�u�p�X�̃������A�N�Z�X�ő҂̂��ݒ肷��
	RenderPassBuilder withDstAccessMask(const VkAccessFlags& mask);
	//���̃T�u�p�X����J�ڂ���͈͂�ݒ肷��
	RenderPassBuilder withFlag(const VkDependencyFlags& flag);
	//�T�u�p�X�̈ˑ��֌W��ǉ�����
	RenderPassBuilder addDependency();
	//�T�u�p�X��ǉ�����
	RenderPassBuilder addSubpass();

	//�v���p�e�B����VkRenderPassCreateInfo��Ԃ�
	RenderPassProperty Build();

	//VkRenderPass���쐬����
	void Create(const RenderPassProperty& p, VkRenderPass& pass);
};