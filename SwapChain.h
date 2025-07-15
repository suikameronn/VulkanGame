#pragma once

#include"VulkanCore.h"
#include"TextureFactory.h"
#include"FrameBufferFactory.h"
#include"GpuRenderPassFactory.h"

class SwapChain
{
private:

	uint32_t frameIndex;

	std::shared_ptr<VulkanCore> vulkanCore;
	std::shared_ptr<TextureFactory> textureFactory;
	std::shared_ptr<GpuRenderPassFactory> renderPassFactory;
	std::shared_ptr<FrameBufferFactory> frameBufferFactory;

	VkPhysicalDevice physicalDevice;

	VkDevice device;

	GLFWwindow* window;

	//実行ウィンドウのウィンドウハンドル
	VkSurfaceKHR surface;

	VkSwapchainKHR swapChain;

	std::vector<std::shared_ptr<Texture>> swapChainImages;

	VkFormat swapChainImageFormat;

	VkExtent2D swapChainExtent;

	//スワップチェーンのカラーアタッチメント
	std::shared_ptr<Texture> colorAttachment;

	//スワップチェーンのデプスアタッチメント
	std::shared_ptr<Texture> depthAttachment;

	//フレームバッファ
	std::vector<std::shared_ptr<FrameBuffer>> frameBuffers;

	//スワップチェーンが使用するフォーマットの候補を取得する
	SwapChainSupportDetails querySwapChainSupport();

	//サーフェスのサポートするフォーマットからスワップチェーンのフォーマットを選択する
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	//サーフェスのサポートするチェーンの切り替え方法からスワップチェーンの切り替え方法を選択する
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	//サーフェスのサポートする拡張からスワップチェーンの拡張を選択する
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	//スワップチェーンを作成する
	void createSwapChain();

	//VkSwapchainKHRオブジェクトを作成
	void createSwapChainObj();

	//スワップチェーンのアタッチメントを作成
	void createSwapChainAttachments();

	//カラーアタッチメントを作成
	void createColorAttachment();

	//デプスアタッチメントを作成
	void createDepthAttachment();

	//フレームバッファを作成
	void createFrameBuffers();

	//ウィンドウサイズ変更時のスワップチェーンの再作成
	void recreateSwapChain();

	//スワップチェーンの破棄
	void destroySwapChain();

public:

	SwapChain(std::shared_ptr<VulkanCore> core, std::shared_ptr<TextureFactory> tf
		, std::shared_ptr<GpuRenderPassFactory> rf, std::shared_ptr<FrameBufferFactory> fb);
	
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

	//現在のフレームバッファを取得する
	std::shared_ptr<FrameBuffer> getCurrentFrameBuffer();
};