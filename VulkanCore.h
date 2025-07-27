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

	//画面の画像のフォーマット
	VkFormat swapChainFormat;

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

	//ウィンドウサイズ変更時のコールバックを登録する
	void setupCallBack();

	//レンダー先の出力先のウィンドウの用意
	void createSurface();

	//物理デバイスの選択
	void pickPhysicalDevice();

	//適切なデバイスを選択する
	bool isDeviceSuitable(const VkPhysicalDevice& device);

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

	//ウィンドウサイズの変更のフラグ
	bool windowSizeChanged;

	VulkanCore(GLFWwindow* w);

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

	VkQueue getGraphicsQueue()
	{
		return graphicsQueue;
	}

	VkQueue getPresentQueue()
	{
		return presentQueue;
	}

	bool isShouldCloseWindow()
	{
		return glfwWindowShouldClose(window);
	}

	//コマンドプールを取得する
	VkCommandPool getCommandPool() { return commandPool; }

	//デバイスの指定された機能を持つキューを調べる
	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);

	//画面の画像フォーマットを設定する
	void setSwapChainFormat(VkFormat format);

	//画面の画像フォーマットを取得する
	VkFormat getSwapChainFormat();

	//画面のサーフェスを取得する
	VkSurfaceKHR getSurface();

	//条件に合うメモリのインデックスを取得する
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//使い捨てのコマンドバッファを作成
	VkCommandBuffer& beginSingleTimeCommandBuffer();

	//使い捨てのコマンドバッファを破棄
	void endSingleTimeCommandBuffer(VkCommandBuffer& commandBuffer);

	//GLFWからウィンドウのサイズ変更の通知を受け取る
	void windowResizeCallback(GLFWwindow* window);

	//ウィンドウのサイズ変更を通知する
	bool isWindowSizeChanged();
};

//ウィンドウサイズを変えた時に呼び出され、次回フレームレンダリング前に、スワップチェーンの画像サイズをウィンドウに合わせる
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<VulkanCore*>(glfwGetWindowUserPointer(window));
	app->windowSizeChanged = true;
}