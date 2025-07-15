#pragma once

#include<memory>
#include<iostream>
#include<array>
#include<vector>
#include<list>
#include<string>
#include <optional>
#include<set>

#include<vulkan/vulkan.h>

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

//gpu��̃G���[�̃��b�Z�[�W��\��
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
};

//gpu�ɋ��߂�L���[�̍\����
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//�X���b�v�`�F�[���ɋ��߂�ݒ�̍\����
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanCore
{
private:

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	//�����f�o�C�X
	VkPhysicalDevice physicalDevice;
	//�_���f�o�C�X
	VkDevice device;

	//Vulkan�p�̃C���X�^���X
	VkInstance instance;
	//�f�o�b�O�p
	VkDebugUtilsMessengerEXT debugMessenger;

	//GLFW���̃E�B���h�E�̃f�[�^
	GLFWwindow* window;
	//�E�B���h�E�̃f�[�^
	VkSurfaceKHR surface;

	//�����Ă���f�o�C�X�̍ő�̃}���`�T���v�����O��
	VkSampleCountFlagBits maxMsaaSamples;

	//�����_�����O�p�̃L���[
	VkQueue graphicsQueue;
	//�E�B���h�E�ɉ摜��\�����邽�߂̃L���[
	VkQueue presentQueue;

	//�R�}���h�o�b�t�@�̃v�[��
	VkCommandPool commandPool;

	//Vulkan�C���X�^���X�̍쐬
	void createInstance();

	//�f�o�b�O���C���[�̎g�p�̉ۂ��m�F
	bool checkValidationLayerSupport();

	//glfw��f�o�b�O���C���[�ɕK�v�Ȋg���𒲂ׂ�
	std::vector<const char*> getRequiredExtensions();

	//Vulkan�̃f�o�b�K�̗p��
	void setupDebugMessenger();

	//�E�B���h�E�T�C�Y�ύX���̃R�[���o�b�N��o�^����
	void setupCallBack();

	//�����_�[��̏o�͐�̃E�B���h�E�̗p��
	void createSurface();

	//�����f�o�C�X�̑I��
	void pickPhysicalDevice();

	//�K�؂ȃf�o�C�X��I������
	bool isDeviceSuitable(const VkPhysicalDevice& device);

	//���̃f�o�C�X���K�v�Ȋg���������ǂ������m�F����
	bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);

	//�X���b�v�`�F�[�����쐬�\�ȃf�o�C�X���ǂ����𒲂ׂ�
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device);

	//�f�o�C�X��������ő�̃}���`�T���v�����O���𒲂ׂ�
	VkSampleCountFlagBits getMaxUsableSampleCount();

	//�_���f�o�C�X�̍쐬
	void createLogicalDevice();

	//�R�}���h�v�[���̍쐬
	void createCommandPool();

public:

	//�E�B���h�E�T�C�Y�̕ύX�̃t���O
	bool windowSizeChanged;

	VulkanCore();

	VkPhysicalDevice& getPhysicalDevice()
	{
		return physicalDevice;
	}

	VkDevice& getLogicDevice()
	{
		return device;
	}

	GLFWwindow* getWindow()
	{
		return window;
	}

	const VkSampleCountFlagBits & getMaxMsaaSamples() const
	{
		return maxMsaaSamples;
	}

	VkQueue geGraphicsQueue()
	{
		return graphicsQueue;
	}

	//�R�}���h�v�[�����擾����
	VkCommandPool getCommandPool() { return commandPool; }

	//�f�o�C�X�̎w�肳�ꂽ�@�\�����L���[�𒲂ׂ�
	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);

	//��ʂ̉摜�t�H�[�}�b�g���擾����
	VkFormat getSwapChainFormat();

	//�����ɍ����������̃C���f�b�N�X���擾����
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//�g���̂ẴR�}���h�o�b�t�@���쐬
	VkCommandBuffer& beginSingleTimeCommandBuffer();

	//�g���̂ẴR�}���h�o�b�t�@��j��
	void endSingleTimeCommandBuffer(VkCommandBuffer& commandBuffer);

	//GLFW����E�B���h�E�̃T�C�Y�ύX�̒ʒm���󂯎��
	void windowResizeCallback(GLFWwindow* window);

	//�E�B���h�E�̃T�C�Y�ύX��ʒm����
	bool isWindowSizeChanged();
};

//�E�B���h�E�T�C�Y��ς������ɌĂяo����A����t���[�������_�����O�O�ɁA�X���b�v�`�F�[���̉摜�T�C�Y���E�B���h�E�ɍ��킹��
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<VulkanCore*>(glfwGetWindowUserPointer(window));
	app->windowSizeChanged = true;
}