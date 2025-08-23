#pragma once

#include<memory>
#include<array>

#include"RenderPassFactory.h"
#include"DescriptorSetLayoutFactory.h"
#include"FrameBufferFactory.h"
#include"SwapChain.h"
#include"GpuBufferFactory.h"
#include"VulkanCore.h"

#include<glm/glm.hpp>

struct RenderProperty
{
	VkRenderPassBeginInfo info;

	std::shared_ptr<RenderPass> renderPass;

	std::shared_ptr<FrameBuffer> frameBuffer;

	uint32_t commandIndex;
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

		commandIndex = 0;
	};
};

class Render : public std::enable_shared_from_this<Render>
{
private:

	VkDevice device;

	RenderProperty property;

public:

	Render(VkDevice& d);

	~Render()
	{
#ifdef _DEBUG
		std::cout << "Render :: �f�X�g���N�^" << std::endl;
#endif
	}

	std::shared_ptr<Render> initProperty();

	//�����_�[�p�X��ݒ�
	std::shared_ptr<Render> withRenderPass(const std::shared_ptr<RenderPass> pass);

	//�t���[���o�b�t�@�̐ݒ�
	std::shared_ptr<Render> withFrameBuffer(const std::shared_ptr<FrameBuffer> frameBuffer);

	//�����_�[�͈͂̐ݒ�
	std::shared_ptr<Render> withRenderArea(const uint32_t& width, const uint32_t& height);

	//�����_�[�摜�̏������F�̐ݒ�
	std::shared_ptr<Render> withClearColor(const glm::ivec4& color);

	//�����_�[�摜�̏������̃f�v�X�̐ݒ�
	std::shared_ptr<Render> withClearDepth(const float& depth);

	//�����_�[�摜�̏������̃X�e���V���̐ݒ�
	std::shared_ptr<Render> withClearStencil(const uint32_t& stencil);

	//�R�}���h��ςރR�}���h�o�b�t�@��ݒ�
	std::shared_ptr<Render> withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer);
	std::shared_ptr<Render> withCommandBuffer(const std::shared_ptr<CommandBuffer> commandBuffer, const uint32_t& index);

	//�v���p�e�B��Ԃ�
	RenderProperty Build();

	//�����_�����O�J�n
	void RenderStart(const RenderProperty& property);

	//���ۂ̃����_�����O�̃R�}���h�̐ςݍ��݂�
	//ecsManager��RunFunction���g��

	//�����_�����O�I��
	void RenderEnd(const RenderProperty& property);
};