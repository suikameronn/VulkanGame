#pragma once

#include"VulkanCore.h"
#include"TextureFactory.h"
#include"FrameBufferFactory.h"
#include"RenderPassFactory.h"
#include"CommandBufferFactory.h"

class SwapChain
{
private:

	uint32_t frameIndex;

	std::shared_ptr<VulkanCore> vulkanCore;
	std::shared_ptr<TextureFactory> textureFactory;
	std::shared_ptr<RenderPassFactory> renderPassFactory;
	std::shared_ptr<FrameBufferFactory> frameBufferFactory;
	std::shared_ptr<CommandBufferFactory> commandBufferFactory;

	VkPhysicalDevice physicalDevice;

	VkDevice device;

	GLFWwindow* window;

	//���s�E�B���h�E�̃E�B���h�E�n���h��
	VkSurfaceKHR surface;

	VkSwapchainKHR swapChain;

	std::vector<std::shared_ptr<Texture>> swapChainImages;

	VkFormat swapChainImageFormat;

	VkExtent2D swapChainExtent;

	uint32_t availableSwapChaneImageNumber;

	//�X���b�v�`�F�[���̃J���[�A�^�b�`�����g
	std::shared_ptr<Texture> colorAttachment;

	//�X���b�v�`�F�[���̃f�v�X�A�^�b�`�����g
	std::shared_ptr<Texture> depthAttachment;

	//�t���[���o�b�t�@
	std::vector<std::shared_ptr<FrameBuffer>> frameBuffers;

	//���̃R�}���h�o�b�t�@���g�p�\�ɂȂ�܂ő҂�
	std::vector<VkSemaphore> imageAvailableSemaphores;

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

	//�����p�̃��\�[�X���쐬
	void createSync();

	//�X���b�v�`�F�[���̔j��
	void destroySwapChain();

	//�Z�}�t�H�̔j��
	void destroySemaphore();

public:

	SwapChain(std::shared_ptr<VulkanCore> core, std::shared_ptr<TextureFactory> tf
		, std::shared_ptr<RenderPassFactory> rf, std::shared_ptr<FrameBufferFactory> fb
		, std::shared_ptr<CommandBufferFactory> cf);
	
	~SwapChain()
	{
		destroySwapChain();

		vkDestroySurfaceKHR(vulkanCore->getInstance(), surface, nullptr);

		destroySemaphore();

#ifdef _DEBUG
		std::cout << "SwapChain :: �f�X�g���N�^" << std::endl;
#endif
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

	//���݂̃t���[���C���f�b�N�X���擾����
	uint32_t getCurrentFrameIndex() const
	{
		return frameIndex;
	}

	//�摜�����p�\�ɂȂ�܂ő҂Z�}�t�H���擾����
	VkSemaphore& getImageAvailableSemaphore()
	{
		return imageAvailableSemaphores[frameIndex];
	}

	//���݂̃t���[���o�b�t�@���擾����
	std::shared_ptr<FrameBuffer> getCurrentFrameBuffer();

	//���C���̃����_�����O���I����
	//�X���b�v�`�F�[���̉摜��؂�ւ���(�������s��)
	void flipSwapChainImage(std::shared_ptr<CommandBuffer> commandBuffer);
};