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

	//実行ウィンドウのウィンドウハンドル
	VkSurfaceKHR surface;

	VkSwapchainKHR swapChain;

	std::vector<std::shared_ptr<Texture>> swapChainImages;

	VkFormat swapChainImageFormat;

	VkExtent2D swapChainExtent;

	uint32_t availableSwapChaneImageNumber;

	//スワップチェーンのカラーアタッチメント
	std::shared_ptr<Texture> colorAttachment;

	//スワップチェーンのデプスアタッチメント
	std::shared_ptr<Texture> depthAttachment;

	//フレームバッファ
	std::vector<std::shared_ptr<FrameBuffer>> frameBuffers;

	//そのコマンドバッファが使用可能になるまで待つ
	std::vector<VkSemaphore> imageAvailableSemaphores;

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

	//同期用のリソースを作成
	void createSync();

	//スワップチェーンの破棄
	void destroySwapChain();

	//セマフォの破棄
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
		std::cout << "SwapChain :: デストラクタ" << std::endl;
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

	//現在のフレームインデックスを取得する
	uint32_t getCurrentFrameIndex() const
	{
		return frameIndex;
	}

	//画像が利用可能になるまで待つセマフォを取得する
	VkSemaphore& getImageAvailableSemaphore()
	{
		return imageAvailableSemaphores[frameIndex];
	}

	//現在のフレームバッファを取得する
	std::shared_ptr<FrameBuffer> getCurrentFrameBuffer();

	//メインのレンダリングを終える
	//スワップチェーンの画像を切り替える(同期も行う)
	void flipSwapChainImage(std::shared_ptr<CommandBuffer> commandBuffer);
};