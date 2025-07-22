#pragma once

#include"RenderPassFactory.h"
#include"DescriptorSetLayoutFactory.h"
#include"FrameBufferFactory.h"
#include"SwapChain.h"
#include"GpuBufferFactory.h"
#include"VulkanCore.h"

#include"ECSManager.h"

#include"GltfModelComp.h"
#include"GltfModelFactory.h"

#include<glm/glm.hpp>

struct RenderProperty
{
	VkRenderPassBeginInfo info;

	std::shared_ptr<RenderPass> renderPass;

	std::shared_ptr<FrameBuffer> frameBuffer;

	std::shared_ptr<CommandBuffer> commandBuffer;

	bool isClear;
	std::array<VkClearValue, 2> clearValues;

	void initProperty()
	{
		info = VkRenderPassBeginInfo{};

		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		info.renderArea.offset = { 0,0 };

		renderPass.reset();

		frameBuffer.reset();

		isClear = false;

		clearValues[0].color = { {0.0f,0.0f,0.0f,1.0f} };
		clearValues[1].depthStencil = { 1.0f,0 };
	}
};

class Render
{
private:
	VkDevice device;

	std::shared_ptr<VulkanCore> vulkanCore;

	RenderProperty property;

	std::shared_ptr<GltfModelFactory> modelFactory;

	std::shared_ptr<ECSManager> ecsManager;

public:

	Render(std::shared_ptr<VulkanCore> core, std::shared_ptr<GpuBufferFactory> bf
		, std::shared_ptr<GltfModelFactory> model, std::shared_ptr<ECSManager> ecs);

	Render initProperty();

	//�����_�[�p�X��ݒ�
	Render withRenderPass(const std::shared_ptr<RenderPass> pass);

	//�t���[���o�b�t�@�̐ݒ�
	Render withFrameBuffer(const std::shared_ptr<FrameBuffer> frameBuffer);

	//�����_�[�͈͂̐ݒ�
	Render withRenderArea(const uint32_t& width, const uint32_t& height);

	//�����_�[�摜�̏������F�̐ݒ�
	Render withClearColor(const glm::ivec4& color);

	//�����_�[�摜�̏������̃f�v�X�̐ݒ�
	Render withClearDepth(const float& depth);

	//�����_�[�摜�̏������̃X�e���V���̐ݒ�
	Render withClearStencil(const uint32_t& stencil);

	//�R�}���h��ςރR�}���h�o�b�t�@��ݒ�
	Render withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer);

	//�v���p�e�B��Ԃ�
	RenderProperty Build();

	//�����_�����O�J�n
	void RenderStart(const RenderProperty& property);

	//�����_�����O
	void Rendering();

	//�����_�����O�I��
	void RenderEnd();
};