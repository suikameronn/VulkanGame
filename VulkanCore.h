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

//gpu上のエラーのメッセージを表示
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
};

//gpuに求めるキューの構造体
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//スワップチェーンに求める設定の構造体
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

	//物理デバイス
	VkPhysicalDevice physicalDevice;
	//論理デバイス
	VkDevice device;

	//Vulkan用のインスタンス
	VkInstance instance;
	//デバッグ用
	VkDebugUtilsMessengerEXT debugMessenger;

	//GLFW側のウィンドウのデータ
	GLFWwindow* window;
	//ウィンドウのデータ
	VkSurfaceKHR surface;

	//扱っているデバイスの最大のマルチサンプリング数
	VkSampleCountFlagBits maxMsaaSamples;

	//レンダリング用のキュー
	VkQueue graphicsQueue;
	//ウィンドウに画像を表示するためのキュー
	VkQueue presentQueue;

	//コマンドバッファのプール
	VkCommandPool commandPool;

	//Vulkanインスタンスの作成
	void createInstance();

	//デバッグレイヤーの使用の可否を確認
	bool checkValidationLayerSupport();

	//glfwやデバッグレイヤーに必要な拡張を調べる
	std::vector<const char*> getRequiredExtensions();

	//Vulkanのデバッガの用意
	void setupDebugMessenger();

	//レンダー先の出力先のウィンドウの用意
	void createSurface();

	//物理デバイスの選択
	void pickPhysicalDevice();

	//適切なデバイスを選択する
	bool isDeviceSuitable(const VkPhysicalDevice& device);

	//デバイスの指定された機能を持つキューを調べる
	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);

	//そのデバイスが必要な拡張を持つかどうかを確認する
	bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);

	//スワップチェーンを作成可能なデバイスかどうかを調べる
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device);

	//デバイスが扱える最大のマルチサンプリング数を調べる
	VkSampleCountFlagBits getMaxUsableSampleCount();

	//論理デバイスの作成
	void createLogicalDevice();

	//コマンドプールの作成
	void createCommandPool();

public:

	VulkanCore();

	VkPhysicalDevice& getPhysicalDevice()
	{
		return physicalDevice;
	}

	VkDevice& getLogicDevice()
	{
		return device;
	}

	//画面の画像フォーマットを取得する
	VkFormat getSwapChainFormat();

	//使い捨てのコマンドバッファを作成
	VkCommandBuffer& beginSingleTimeCommandBuffer();

	//使い捨てのコマンドバッファを破棄
	void endSingleTimeCommandBuffer(VkCommandBuffer& commandBuffer);
};