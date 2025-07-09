#pragma once

#include<algorithm>

#include"VulkanCore.h"

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
};

class GpuRenderPassBuilder
{
private:

	//�_���f�o�C�X
	VkDevice device;

	RenderPassProperty property;

	VkAttachmentDescription description;

	VkSubpassDependency dependency;

	VkSubpassDescription subpassDescription;

public:

	GpuRenderPassBuilder(VkDevice& d);

	//�v���p�e�B������������
	void initProperty();

	//VkAttachmentDescription�̍쐬

	//�t�H�[�}�b�g�̐ݒ�
	GpuRenderPassBuilder withFormat(const VkFormat& format);
	//�}���`�T���v�����O�̃T���v������ݒ�
	GpuRenderPassBuilder withMultiSamples(const VkSampleCountFlagBits& count);
	//�J���[�A�^�b�`�����g���g�p����O�̏������w��
	GpuRenderPassBuilder withColorLoadOp(const VkAttachmentLoadOp& op);
	//�J���[�A�^�b�`�����g���g�p������̏������w��
	GpuRenderPassBuilder withColorStoreOp(const VkAttachmentStoreOp& op);
	//�X�e���V���A�^�b�`�����g���g�p����O�̏������w��
	GpuRenderPassBuilder withStencilLoadOp(const VkAttachmentLoadOp& op);
	//�X�e���V���A�^�b�`�����g���g�p������̏������w��
	GpuRenderPassBuilder withStencilStoreOp(const VkAttachmentStoreOp& op);
	//�T�u�p�X�ɓ���O�̃��C�A�E�g���w��
	GpuRenderPassBuilder withInitialLayout(const VkImageLayout& layout);
	//�T�u�p�X�ɏo����̃��C�A�E�g���w��
	GpuRenderPassBuilder withFinalLayout(const VkImageLayout& layout);
	//�f�B�X�N���v�V������ǉ�����
	GpuRenderPassBuilder addColorAttachment();
	GpuRenderPassBuilder addColorResolveAttachment();
	GpuRenderPassBuilder addDepthStencilAttachment();

	//VkSubpassDependency�̍쐬

	//��O�̃T�u�p�X���w�肷��
	GpuRenderPassBuilder withSrcSubpassIndex(const uint32_t& index);
	//���̃T�u�p�X���w�肷��
	GpuRenderPassBuilder withDstSubpassIndex(const uint32_t& index);
	//��O�̃T�u�p�X���ǂ̃X�e�[�W�܂ōs���܂ő҂���ݒ肷��
	GpuRenderPassBuilder withSrcStageMask(const VkPipelineStageFlags& flag);
	//��O�̃T�u�p�X�̃������A�N�Z�X��҂̂��ݒ肷��
	GpuRenderPassBuilder withSrcAccessMask(const VkAccessFlags& mask);
	//���̃T�u�p�X���ǂ̃X�e�[�W�ő҂���ݒ肷��
	GpuRenderPassBuilder withDstStageMask(const VkPipelineStageFlags& flag);
	//���̃T�u�p�X�̃������A�N�Z�X�ő҂̂��ݒ肷��
	GpuRenderPassBuilder withDstAccessMask(const VkAccessFlags& mask);
	//���̃T�u�p�X����J�ڂ���͈͂�ݒ肷��
	GpuRenderPassBuilder withFlag(const VkDependencyFlags& flag);
	//�T�u�p�X�̈ˑ��֌W��ǉ�����
	GpuRenderPassBuilder addDependency();
	//�T�u�p�X��ǉ�����
	GpuRenderPassBuilder addSubpass();

	//�v���p�e�B����VkRenderPassCreateInfo��Ԃ�
	RenderPassProperty Build();

	//VkRenderPass���쐬����
	void Create(const RenderPassProperty& p, VkRenderPass& pass);
};