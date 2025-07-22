#pragma once

#include"VulkanCore.h"
#include"TextureFactory.h"
#include"FrameBufferFactory.h"
#include"RenderPassFactory.h"

class SwapChain
{
private:

	uint32_t frameIndex;

	std::shared_ptr<VulkanCore> vulkanCore;
	std::shared_ptr<TextureFactory> textureFactory;
	std::shared_ptr<RenderPassFactory> renderPassFactory;
	std::shared_ptr<FrameBufferFactory> frameBufferFactory;

	VkPhysicalDevice physicalDevice;

	VkDevice device;

	GLFWwindow* window;

	//���s�E�B���h�E�̃E�B���h�E�n���h��
	VkSurfaceKHR surface;

	VkSwapchainKHR swapChain;

	std::vector<std::shared_ptr<Texture>> swapChainImages;

	VkFormat swapChainImageFormat;

	VkExtent2D swapChainExtent;

	//�X���b�v�`�F�[���̃J���[�A�^�b�`�����g
	std::shared_ptr<Texture> colorAttachment;

	//�X���b�v�`�F�[���̃f�v�X�A�^�b�`�����g
	std::shared_ptr<Texture> depthAttachment;

	//�t���[���o�b�t�@
	std::vector<std::shared_ptr<FrameBuffer>> frameBuffers;

	//�X���b�v�`�F�[�����g�p����t�H�[�}�b�g�̌����擾����
	SwapChainSupportDetails querySwapChainSupport();

	//�T�[�t�F�X�̃T�|�[�g����t�H�[�}�b�g����X���b�v�`�F�[���̃t�H�[�}�b�g��I������
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	//�T�[�t�F�X�̃T�|�[�g����`�F�[���̐؂�ւ����@����X���b�v�`�F�[���̐؂�ւ����@��I������
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	//�T�[�t�F�X�̃T�|�[�g����g������X���b�v�`�F�[���̊g����I������
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	//�X���b�v�`�F�[�����쐬����
	void createSwapChain();

	//VkSwapchainKHR�I�u�W�F�N�g���쐬
	void createSwapChainObj();

	//�X���b�v�`�F�[���̃A�^�b�`�����g���쐬
	void createSwapChainAttachments();

	//�J���[�A�^�b�`�����g���쐬
	void createColorAttachment();

	//�f�v�X�A�^�b�`�����g���쐬
	void createDepthAttachment();

	//�t���[���o�b�t�@���쐬
	void createFrameBuffers();

	//�E�B���h�E�T�C�Y�ύX���̃X���b�v�`�F�[���̍č쐬
	void recreateSwapChain();

	//�X���b�v�`�F�[���̔j��
	void destroySwapChain();

public:

	SwapChain(std::shared_ptr<VulkanCore> core, std::shared_ptr<TextureFactory> tf
		, std::shared_ptr<RenderPassFactory> rf, std::shared_ptr<FrameBufferFactory> fb);
	
	~SwapChain()
	{
		destroySwapChain();
	}

	VkSwapchainKHR getSwapchin()
	{
		return swapChain;
	}

	const VkExtent2D& getSwapChainExtent() const
	{
		return swapChainExtent;
	}

	const VkFormat& getSwapChainImageFormat() const
	{
		return swapChainImageFormat;
	}

	//���݂̃t���[���o�b�t�@���擾����
	std::shared_ptr<FrameBuffer> getCurrentFrameBuffer();
};