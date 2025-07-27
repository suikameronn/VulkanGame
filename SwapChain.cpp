#include"SwapChain.h"

SwapChain::SwapChain(std::shared_ptr<VulkanCore> core, std::shared_ptr<TextureFactory> tf
    , std::shared_ptr<RenderPassFactory> rf, std::shared_ptr<FrameBufferFactory> fb)
{
    frameIndex = 0;

    vulkanCore = core;
    textureFactory = tf;
	renderPassFactory = rf;
	frameBufferFactory = fb;

    physicalDevice = vulkanCore->getPhysicalDevice();
    device = vulkanCore->getLogicDevice();
    window = vulkanCore->getWindow();
    surface = vulkanCore->getSurface();

	createSwapChain();
}

//�X���b�v�`�F�[�����쐬����
void SwapChain::createSwapChain()
{
	//VkSwapchainKHR�I�u�W�F�N�g���쐬
    createSwapChainObj();

    //�X���b�v�`�F�[���̃A�^�b�`�����g���쐬
	createSwapChainAttachments();

    //�t���[���o�b�t�@���쐬
    createFrameBuffers();
}

//�T�[�t�F�X����X���b�v�`�F�[���̃T�|�[�g�����擾
SwapChainSupportDetails SwapChain::querySwapChainSupport() 
{
    SwapChainSupportDetails details;

	//�T�[�t�F�X����X���b�v�`�F�[���̃T�|�[�g�����擾
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
	//�T�[�t�F�X���T�|�[�g����t�H�[�}�b�g�̐����擾
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);

		//�T�[�t�F�X���T�|�[�g����t�H�[�}�b�g�̏����擾
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
	//�T�[�t�F�X���T�|�[�g����v���[���g���[�h�̐����擾
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);

		//�T�[�t�F�X���T�|�[�g����v���[���g���[�h�̏����擾
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

//�T�[�t�F�X�̃T�|�[�g����t�H�[�}�b�g����X���b�v�`�F�[���̃t�H�[�}�b�g��I������
VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

//�T�[�t�F�X�̃T�|�[�g����`�F�[���̐؂�ւ����@����X���b�v�`�F�[���̐؂�ւ����@��I������
VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

//�T�[�t�F�X�̃T�|�[�g����g������X���b�v�`�F�[���̊g����I������
VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

//VkSwapchainKHR�I�u�W�F�N�g���쐬
void SwapChain::createSwapChainObj()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = vulkanCore->findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    std::vector<VkImage> swapChainImageHandles(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImageHandles.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    vulkanCore->setSwapChainFormat(swapChainImageFormat);

    //�X���b�v�`�F�[����VkImage�̃r���[���쐬

    TextureProperty property = textureFactory->getBuilder()
        ->withViewType(VK_IMAGE_VIEW_TYPE_2D)
        .withFormat(swapChainImageFormat)
        .withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
        .withLayerCount(1)
        .Build();

    swapChainImages.resize(imageCount);
    for (size_t i = 0; i < swapChainImages.size(); ++i)
    {
        swapChainImages[i] = textureFactory->ViewCreate(property, swapChainImageHandles[i]);
    }
}

//�X���b�v�`�F�[���̃A�^�b�`�����g���쐬
void SwapChain::createSwapChainAttachments()
{
	createColorAttachment();

	createDepthAttachment();
}

//�J���[�A�^�b�`�����g���쐬
void SwapChain::createColorAttachment()
{
	const VkSampleCountFlagBits msaaSampleCount = vulkanCore->getMaxMsaaSamples();

    TextureProperty property = textureFactory->getBuilder()->initProperty()
        .withWidthHeight(swapChainExtent.width, swapChainExtent.height, 1)
        .withFormat(swapChainImageFormat)
        .withNumSamples(msaaSampleCount)
        .withTiling(VK_IMAGE_TILING_OPTIMAL)
        .withUsage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        .withViewType(VK_IMAGE_VIEW_TYPE_2D)
        .withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
        .withLayerCount(1)
        .Build();

    colorAttachment = textureFactory->ImageViewCreate(property);
}

//�f�v�X�A�^�b�`�����g���쐬
void SwapChain::createDepthAttachment()
{
    const VkSampleCountFlagBits msaaSampleCount = vulkanCore->getMaxMsaaSamples();

    TextureProperty property = textureFactory->getBuilder()->initProperty()
        .withWidthHeight(swapChainExtent.width, swapChainExtent.height, 1)
        .withFormat(VK_FORMAT_D32_SFLOAT_S8_UINT)
        .withNumSamples(msaaSampleCount)
        .withTiling(VK_IMAGE_TILING_OPTIMAL)
        .withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        .withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        .withViewType(VK_IMAGE_VIEW_TYPE_2D)
        .withViewAccess(VK_IMAGE_ASPECT_DEPTH_BIT)
        .withLayerCount(1)
        .Build();

    depthAttachment = textureFactory->ImageViewCreate(property);
}

//�t���[���o�b�t�@���쐬
void SwapChain::createFrameBuffers()
{
	frameBuffers.resize(swapChainImages.size());

    std::shared_ptr<RenderPass> renderPass = renderPassFactory->Create(RenderPassPattern::PBR);

    for (size_t i = 0; i < frameBuffers.size(); i++) 
    {
        const FrameBufferProperty property = frameBufferFactory->getBuilder()->initProperty()
            .withWidthHeight(swapChainExtent.width, swapChainExtent.height)
            .withRenderPass(renderPass)
            .withLayerCount(1)
			.addViewAttachment(colorAttachment)
            .addViewAttachment(depthAttachment)
			.addViewAttachment(swapChainImages[i])
			.Build();

        frameBuffers[i] = frameBufferFactory->Create(property);
    }
}

//�X���b�v�`�F�[���̉摜��؂�ւ���(���@���s��)
void SwapChain::flipSwapChainImage(std::shared_ptr<CommandBuffer> commandBuffer)
{
    if (vkEndCommandBuffer(commandBuffer->commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkSemaphore> waitSemaphores = { imageAvailableSemaphores[currentFrame] };
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

    std::vector<VkSemaphore> signalSemaphores = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    if (vkQueueSubmit(vulkanCore->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores.data();

    std::vector<VkSwapchainKHR> swapChains = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains.data();

    presentInfo.pImageIndices = &availableSwapChaneImageNumber;

    VkResult result;
    result = vkQueuePresentKHR(vulkanCore->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkanCore->isWindowSizeChanged()) 
    {
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame == 1) ? 0 : 1;
}

//�X���b�v�`�F�[���̔j��
void SwapChain::destroySwapChain()
{
    swapChainImages.clear();

	colorAttachment.reset();

	depthAttachment.reset();

    frameBuffers.clear();

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

//�E�B���h�E�T�C�Y�ύX���̃X���b�v�`�F�[���̍č쐬
void SwapChain::recreateSwapChain()
{
    //�E�B���h�E�T�C�Y���ύX���ꂽ�ꍇ�A�X���b�v�`�F�[�����č쐬����

    destroySwapChain();

    createSwapChain();
}

//���݂̃t���[���o�b�t�@���擾����
std::shared_ptr<FrameBuffer> SwapChain::getCurrentFrameBuffer()
{
    frameIndex = (frameIndex == 0) ? 1 : 0;

    return frameBuffers[frameIndex];
}

