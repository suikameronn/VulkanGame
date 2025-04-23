#include"VulkanBase.h"

VulkanBase* VulkanBase::vulkanBase = nullptr;

    VkResult VulkanBase::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void VulkanBase::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    //Vulkanの初期化
    void VulkanBase::initVulkan() {
        descriptorSetCount = 0;

        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createCommandPool();
        createColorResources();
        createDepthResources();
        createFramebuffers();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorPool();
        createEmptyImage();

        prepareUIRendering();//ui描画の用意
    }

    //スワップチェーンの破棄
    void VulkanBase::cleanupSwapChain() {
        vkDestroyImageView(device, colorImageView, nullptr);
        vkDestroyImage(device, colorImage, nullptr);
        vkFreeMemory(device, colorImageMemory, nullptr);

        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    //ゲーム終了時にデータのgpu上のデータをすべて破棄する
    void VulkanBase::cleanup()
    {
        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        emptyImage.destroy(device);
        shadowMapData.destroy(device);
        modelDescriptor.destroy(device);
        cubemapData.destroy(device);
        uiRender.destroy(device,multiThreadCommandPool);

        //IBLのデータの破棄
        iblDiffuse.destroy(device);
        iblSpecularReflection.destroy(device);
        iblSpecularBRDF.destroy(device);

        Storage* storage = Storage::GetInstance();

        for (auto gltfModel:storage->getgltfModel())//gltfモデルのバッファーなどの削除
        {
            gltfModel.second->cleanUpVulkan(device);
        }

        storage->getCubeMap()->cleanupVulkan();//キューブマップ用のバッファーなどの削除

        for (auto model:storage->getModels())//Modelクラスのバッファーなどの削除
        {
            model->cleanupVulkan();
        }

        //UIのバッファの破棄
        storage->getLoadUI()->cleanupVulkan();
        for (auto ui : storage->getUI())
        {
            ui->cleanupVulkan();
        }

        storage->getPointLightsBuffer().destroy(device);//ポイントライトのバッファの後処理
        storage->getDirectionalLightsBuffer().destroy(device);//ディレクショナルライトのバッファの後処理

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
            vkDestroyFence(device, multiThreadFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyCommandPool(device, multiThreadCommandPool, nullptr);

        //この時点でgpu側に作った変数を破棄して置かなければならない
        //deviceの変数を使って作る変数も破棄
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    //ウィンドウサイズが変わった際のスワップチェーンの画像サイズの変更
    void VulkanBase::recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createColorResources();
        createDepthResources();
        createFramebuffers();
    }

    //インスタンスの作成
    void VulkanBase::createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void VulkanBase::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void VulkanBase::setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    //レンダー先の出力先のウィンドウの用意
    void VulkanBase::createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    VkSampleCountFlagBits VulkanBase::getMaxUsableSampleCount()
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
            physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    //Vulkanで扱うデバイスを選択する
    void VulkanBase::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                msaaSamples = getMaxUsableSampleCount();
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    //Vulkan上で扱うデバイスデータの作成
    void VulkanBase::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        std::array<float, 2> queuePrioritys = { 1.0f,0.0f };
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 2;//マルチスレッド用のキューも確保する
            queueCreateInfo.pQueuePriorities = queuePrioritys.data();
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 1, &multiThreadGraphicQueue);//インデックスを二つ目に設定
        vkGetDeviceQueue(device, indices.presentFamily.value(), 1, &multiThreadPresentQueue);
    }

    //スワップチェーンの作成
    void VulkanBase::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
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

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void VulkanBase::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i], VK_IMAGE_VIEW_TYPE_2D,swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT,1,1);
        }
    }

    //レンダーパスの作成
    void VulkanBase::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = swapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 3>attachments = { colorAttachment,depthAttachment,colorAttachmentResolve };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    //MVP行列とアニメーション行列を含むdescriptorSetLayoutを作成
    void VulkanBase::createDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout) {
        std::vector<VkDescriptorSetLayoutBinding> bindings(2);
        
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding uboLayoutBindingAnimation{};
        uboLayoutBindingAnimation.binding = 1;
        uboLayoutBindingAnimation.descriptorCount = 1;
        uboLayoutBindingAnimation.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBindingAnimation.pImmutableSamplers = nullptr;
        uboLayoutBindingAnimation.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        bindings[0] = uboLayoutBinding;
        bindings[1] = uboLayoutBindingAnimation;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    //ShaderMaterialを含むdescriptorSetLayoutの作成
    void VulkanBase::createDescriptorSetLayout(std::shared_ptr<Material> material)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings(MAX_TEXTURE_COUNT + 1);

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0] = uboLayoutBinding;

        for (int i = 1; i < MAX_TEXTURE_COUNT + 1; i++)
        {
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = i;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            bindings[i] = samplerLayoutBinding;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &modelDescriptor.materialLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    //通常のレンダリングで使用するパイプラインの作成
    void VulkanBase::createGraphicsPipeline(std::string vertFile,std::string fragFile,VkPrimitiveTopology topology,
        VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline) {

        auto vertShaderCode = readFile(vertFile);
        auto fragShaderCode = readFile(fragFile);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        {
            bindingDescription.stride = sizeof(Vertex);

            attributeDescriptions.resize(7);
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord0);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, texCoord1);

            attributeDescriptions[4].binding = 0;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[4].offset = offsetof(Vertex, normal);

            attributeDescriptions[5].binding = 0;
            attributeDescriptions[5].location = 5;
            attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
            attributeDescriptions[5].offset = offsetof(Vertex, boneID1);

            attributeDescriptions[6].binding = 0;
            attributeDescriptions[6].location = 6;
            attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[6].offset = offsetof(Vertex, weight1);
        }
        else if (topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
        {
            bindingDescription.stride = sizeof(glm::vec3);

            attributeDescriptions.resize(1);

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = 0;
        }

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.minSampleShading = 0.2f;
        multisampling.rasterizationSamples = msaaSamples;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkPushConstantRange pushConstant;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(PushConstantObj);
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        std::vector<VkDescriptorSetLayout> layouts(8);
        layouts[0] = layout;//MVP行列とアニメーション行列
        layouts[1] = modelDescriptor.materialLayout;//マテリアル
        layouts[2] = modelDescriptor.lightLayout;//ポイントライト
        layouts[3] = modelDescriptor.lightLayout;//平行光源
        layouts[4] = shadowMapData.layout;//シャドウマップ
        layouts[5] = iblDiffuse.mainPassLayout;//IBLのdiffuse用
        layouts[6] = iblSpecularReflection.mainPassLayout;//IBLのspecularの鏡面反射用
        layouts[7] = iblSpecularBRDF.mainPassLayout;//IBLのspecularのBRDF用

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    //シャドウマップ作成用のパイプラインを作成
    void VulkanBase::createShadowMapPipeline(std::string vertexPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline,VkRenderPass& pass)
    {
        auto vertShaderCode = readFile(vertexPath);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SINT;
        attributeDescriptions[1].offset = offsetof(Vertex, boneID1);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, weight1);

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_TRUE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.minSampleShading = 0.2f;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 0;//カラーアタッチメントはいらない
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPushConstantRange pushConstant;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(PushConstantObj);
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_DEPTH_BIAS//デプスバッファの値を調整できるようにする
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        std::vector<VkDescriptorSetLayout> layouts(2);
        layouts[0] = layout;
        layouts[1] = modelDescriptor.layout;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 1;
        pipelineInfo.pStages = &vertShaderStageInfo;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pLayout;
        pipelineInfo.renderPass = pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    //HDRI画像からキューブマップ用のパイプラインの作成
    void VulkanBase::createCalcCubeMapPipeline(std::string vertexPath, std::string fragPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass)
    {
        auto vertShaderCode = readFile(vertexPath);
        auto fragShaderCode = readFile(fragPath);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(2);
        shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageInfos[0].module = vertShaderModule;
        shaderStageInfos[0].pName = "main";

        shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageInfos[1].module = fragShaderModule;
        shaderStageInfos[1].pName = "main";

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        VkVertexInputAttributeDescription attributeDescription;
        attributeDescription.binding = 0;
        attributeDescription.location = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(Vertex, pos);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(1);
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.minSampleShading = 0.2f;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        std::vector<VkDescriptorSetLayout> layouts(2);
        layouts[0] = layout;
        layouts[1] = modelDescriptor.layout;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
        pipelineInfo.pStages = shaderStageInfos.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pLayout;
        pipelineInfo.renderPass = pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
    }

    //キューブマップのレンダリング用のパイプラインの作成
    void VulkanBase::createCubemapPipeline(std::string vertexPath, std::string fragPath
        , VkDescriptorSetLayout& layout, VkPipelineLayout& pLayout, VkPipeline& pipeline, VkRenderPass& pass)
    {
        auto vertShaderCode = readFile(vertexPath);
        auto fragShaderCode = readFile(fragPath);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(2);
        shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageInfos[0].module = vertShaderModule;
        shaderStageInfos[0].pName = "main";

        shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageInfos[1].module = fragShaderModule;
        shaderStageInfos[1].pName = "main";

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        VkVertexInputAttributeDescription attributeDescription;
        attributeDescription.binding = 0;
        attributeDescription.location = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(Vertex, pos);

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(1);
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.minSampleShading = 0.2f;
        multisampling.rasterizationSamples = msaaSamples;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        std::vector<VkDescriptorSetLayout> layouts(2);
        layouts[0] = modelDescriptor.layout;
        layouts[1] = layout;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
        pipelineInfo.pStages = shaderStageInfos.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pLayout;
        pipelineInfo.renderPass = pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
    }

    //IBLのマップ計算用のパイプラインの作成
    void VulkanBase::createCalcIBLMapPipeline(std::string vertShaderPath, std::string fragShaderPath, VkDescriptorSetLayout& prePassLayout
        , VkPipelineLayout& pipelineLayout, uint32_t pipelineNum, VkPipeline* pipeline,uint32_t renderPassNum,VkRenderPass* renderPass)
    {
        auto vertShaderCode = readFile(vertShaderPath);
        auto fragShaderCode = readFile(fragShaderPath);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(2);
        shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageInfos[0].module = vertShaderModule;
        shaderStageInfos[0].pName = "main";

        shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageInfos[1].module = fragShaderModule;
        shaderStageInfos[1].pName = "main";

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord1);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.minSampleShading = 0.2f;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        //PipelneLayoutを重複して作成するのを回避する
        if (!pipelineLayout)
        {
            std::vector<VkDescriptorSetLayout> layouts(2);
            layouts[0] = prePassLayout;
            layouts[1] = modelDescriptor.layout;

            VkPushConstantRange push_constant;
            push_constant.offset = 0;
            push_constant.size = sizeof(SpecularPushConstant);
            push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
            pipelineLayoutInfo.pSetLayouts = layouts.data();
            pipelineLayoutInfo.pPushConstantRanges = &push_constant;
            pipelineLayoutInfo.pushConstantRangeCount = 1;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }
        }

        for (uint32_t i = 0; i < pipelineNum;i++)
        {
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
            pipelineInfo.pStages = shaderStageInfos.data();
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.renderPass = renderPass[i];
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDepthStencilState = &depthStencil;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }
        }

        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
    }

    //通常のレンダリングで仕様するフレームバッファの作成
    void VulkanBase::createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 3> attachments = {
                colorImageView,
                depthImageView,
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    //コマンドプールの作成 コマンドバッファーはこれから作る
    void VulkanBase::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }

        //マルチスレッド用も作成する
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &multiThreadCommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    //通常のレンダリングで使用するカラーアタッチメントの作成
    void VulkanBase::createColorResources()
    {
        VkFormat colorFormat = swapChainImageFormat;

        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);

        colorImageView = createImageView(colorImage, VK_IMAGE_VIEW_TYPE_2D, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1,1);
    }

    //通常のレンダリングで使用するデプスアタッチメントの作成
    void VulkanBase::createDepthResources()
    {
        VkFormat depthFormat = findDepthFormat();

        createImage(swapChainExtent.width, swapChainExtent.height,1,msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, VK_IMAGE_VIEW_TYPE_2D, depthFormat,VK_IMAGE_ASPECT_DEPTH_BIT,1,1);

        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,1,1);

    }

    //gpuがサポートしている画像フォーマットの探索
    VkFormat VulkanBase::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format");
    }

    //gpuがサポートしているデプスイメージのフォーマットの探索
    VkFormat VulkanBase::findDepthFormat()
    {
        return findSupportedFormat
        (
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool VulkanBase::hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    //画像のサイズから作成可能なミップマップレベルの計算
    uint32_t VulkanBase::calcMipMapLevel(uint32_t width, uint32_t height)
    {
        return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
    }

    //単一のレイヤーのVkImageを複数のレイヤーのVkImageにコピーする
    void VulkanBase::copyImageToMultiLayerImage(VkImage* srcImages, uint32_t imageCount,uint32_t width,uint32_t height, VkImage& dstImage)
    {
        //コマンドの記録開始
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        //VkImageを枚数分一つのVkImageにコピー
        for (uint32_t i = 0; i < imageCount; i++)
        {
            VkImageCopy region{};
            region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.srcSubresource.mipLevel = 0;
            region.srcSubresource.baseArrayLayer = 0;
            region.srcSubresource.layerCount = 1;
            region.srcOffset = { 0, 0, 0 };
            region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.dstSubresource.mipLevel = 0;
            region.dstSubresource.baseArrayLayer = i;
            region.dstSubresource.layerCount = 1;
            region.dstOffset = { 0, 0, 0 };
            region.extent = { width, height, 1 };

            vkCmdCopyImage(commandBuffer, srcImages[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }

        //コマンドバッファの記録を終了する
        endSingleTimeCommands(commandBuffer);

    }

    //画像からテクスチャ画像の作成 
    void VulkanBase::createTextureImage(TextureData* textureData,std::shared_ptr<ImageData> image,VkFormat format)
    {
        VkDeviceSize bufferSize = image->getWidth() * image->getHeight() * image->getTexChannels() * image->getPixelPerByte();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, image->getPixelsData(), bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        textureData->mipLevel = calcMipMapLevel(image->getWidth(),image->getHeight());
        createImage(image->getWidth(), image->getHeight(), textureData->mipLevel, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureData->image, textureData->memory);

        transitionImageLayout(textureData->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureData->mipLevel,1);
        copyBufferToImage(stagingBuffer, textureData->image, static_cast<uint32_t>(image->getWidth()), static_cast<uint32_t>(image->getHeight()),1);

        generateMipmaps(textureData->image, format, image->getWidth(), image->getHeight(), textureData->mipLevel,1);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //ダミーテクスチャの作成
    void VulkanBase::createTextureImage()
    {
        int emptyTexWidth = 2;
        std::vector<float> pixel;
        pixel.resize(emptyTexWidth * 2);
        std::fill(pixel.begin(), pixel.end(), 0.0f);

        TextureData* textureData = emptyImage.emptyTex;

        VkDeviceSize bufferSize = pixel.size() * 4;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, pixel.data() , bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        textureData->mipLevel = calcMipMapLevel(emptyTexWidth,emptyTexWidth);
        createImage(emptyTexWidth,emptyTexWidth, textureData->mipLevel, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureData->image, textureData->memory);

        transitionImageLayout(textureData->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureData->mipLevel,1);
        copyBufferToImage(stagingBuffer, textureData->image, static_cast<uint32_t>(emptyTexWidth), static_cast<uint32_t>(emptyTexWidth),1);

        generateMipmaps(textureData->image, VK_FORMAT_R8G8B8A8_UNORM, emptyTexWidth, emptyTexWidth, textureData->mipLevel,1);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //gltfモデルのマテリアルのテクスチャの作成
    void VulkanBase::createTextureImage(std::shared_ptr<GltfModel> gltfModel,VkFormat format)
    {
        if (gltfModel->textureDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->textureDatas.size(); i++)
        {
            std::shared_ptr<ImageData> imageData = gltfModel->imageDatas[i];
            TextureData* textureData = gltfModel->textureDatas[i];

            VkDeviceSize imageSize = imageData->getWidth() * imageData->getHeight() * imageData->getTexChannels() * imageData->getPixelPerByte();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, imageData->getPixelsData(), static_cast<size_t>(imageSize));
            vkUnmapMemory(device, stagingBufferMemory);

            textureData->mipLevel = calcMipMapLevel(imageData->getWidth(), imageData->getHeight());
            createImage(imageData->getWidth(), imageData->getHeight(), textureData->mipLevel, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
                , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureData->image, textureData->memory);

            transitionImageLayout(textureData->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureData->mipLevel,1);
            copyBufferToImage(stagingBuffer, textureData->image, static_cast<uint32_t>(imageData->getWidth()), static_cast<uint32_t>(imageData->getHeight()),1);

            generateMipmaps(textureData->image, format, imageData->getWidth(), imageData->getHeight(), textureData->mipLevel,1);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }
    }

    //ミップマップ画像の作成
    void VulkanBase::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels,uint32_t layerCount)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr
                , 0, nullptr,
                1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0,0,0 };
            blit.srcOffsets[1] = { mipWidth,mipHeight,1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = layerCount;
            blit.dstOffsets[0] = {0,0,0};
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1,1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = layerCount;

            vkCmdBlitImage(commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }

    //テクスチャのビューの作成
    void VulkanBase::createTextureImageView(TextureData* textureData,VkFormat format)
    {
        textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
    }

    //ダミーテクスチャ用
    void VulkanBase::createTextureImageView()
    {
        TextureData* textureData = emptyImage.emptyTex;
        textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
    }

    //gltfモデルのテクスチャ用のビューの作成
    void VulkanBase::createTextureImageView(std::shared_ptr<GltfModel> gltfModel,VkFormat format) 
    {
        if (gltfModel->textureDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->textureDatas.size(); i++)
        {
            TextureData* textureData = gltfModel->textureDatas[i];
            textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
        }
    }

    //テクスチャのサンプラーの作成
    void VulkanBase::createTextureSampler(TextureData* textureData)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(textureData->mipLevel);
        samplerInfo.mipLodBias = 0.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureData->sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    //ダミーテクスチャのサンプラーの作成
    void VulkanBase::createTextureSampler()
    {
        TextureData* textureData = emptyImage.emptyTex;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(textureData->mipLevel);
        samplerInfo.mipLodBias = 0.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureData->sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    //gltfモデルのテクスチャのサンプラーの作成
    void VulkanBase::createTextureSampler(std::shared_ptr<GltfModel> gltfModel)
    {
        if (gltfModel->textureDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->textureDatas.size(); i++)
        {
            TextureData* textureData = gltfModel->textureDatas[i];

            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = static_cast<float>(textureData->mipLevel);
            samplerInfo.mipLodBias = 0.0f;

            if (vkCreateSampler(device, &samplerInfo, nullptr, &textureData->sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
    }

    VkImageView VulkanBase::createImageView(VkImage image,VkImageViewType type, VkFormat format,VkImageAspectFlags aspectFlags,uint32_t mipLevels,uint32_t layerCount) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = type;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = layerCount;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void VulkanBase::createImage(uint32_t width, uint32_t height,uint32_t mipLevels,VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
           throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    void VulkanBase::createImageSampler(VkSamplerMipmapMode mipMapMode, VkSamplerAddressMode addressMode
        , VkFilter magFilter, VkFilter minFilter, VkSampler& sampler)
    {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = magFilter;
        info.minFilter = minFilter;
        info.mipmapMode = mipMapMode;
        info.addressModeU = addressMode;
        info.addressModeV = addressMode;
        info.addressModeW = addressMode;
        info.mipLodBias = 0.0f;
        info.maxAnisotropy = 1.0f;
        info.minLod = 0.0f;
        info.maxLod = 1.0f;
        info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        
        if (vkCreateSampler(device, &info, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image sampler");
        }
    }

    //テクスチャのレイアウトを変更する
    void VulkanBase::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,uint32_t mipLevels,uint32_t layerCount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        //レイアウトを出力先に変更する
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        //レイアウトをレンダリングの出力先にする
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        //出力されたテクスチャをシェーダ上で利用できるようにする
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        //デプスイメージ用にする
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        //カラーアタッチメント用のレイアウトを送信用のレイアウトに変更する
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        //カラーアタッチメント用のレイアウトをシェーダで使うためのレイアウトにする
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        //カラーアタッチメント用のレイアウトを表示用のレイアウトにする
        else if(oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = 0;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            //パイプラインの処理のすべてが終了したとき
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        //表示用のレイアウトをカラーアタッチメント用のレイアウトにする
        else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer);
    }

    //ピクセルのバッファーからテクスチャ画像を作成する
    void VulkanBase::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,uint32_t layerCount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    //uiのテクスチャを作成する
    void VulkanBase::createUITexture(TextureData* texture, std::shared_ptr<ImageData> image)
    {
        //gpu上に画像データを展開
        createTextureImage(texture, image,VK_FORMAT_R8G8B8A8_UNORM);

        //このテクスチャのビューを作成
        createTextureImageView(texture, VK_FORMAT_R8G8B8A8_UNORM);

        //サンプラーの作成
        createTextureSampler(texture);
    }

    //Modelクラス用の頂点バッファーを作成する
    void VulkanBase::createVertexBuffer(GltfNode* node,std::shared_ptr<Model> model)
    {
        for(auto mesh : node->meshArray)
        {
            VkDeviceSize bufferSize = sizeof(Vertex) * mesh->vertices.size();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, mesh->vertices.data(), (size_t)bufferSize);
            vkUnmapMemory(device, stagingBufferMemory);

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                model->getPointBufferData()[mesh->meshIndex].vertBuffer, model->getPointBufferData()[mesh->meshIndex].vertHandler);

            //vertexBuffer配列にコピーしていく(vector型)
            copyBuffer(stagingBuffer, model->getPointBufferData()[mesh->meshIndex].vertBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        for (size_t i = 0; i < node->children.size(); i++)
        {
            createVertexBuffer(node->children[i], model);
        }
    }

    //UIクラス用の頂点バッファーを作成する
    void VulkanBase::createVertexBuffer(std::shared_ptr<UI> ui)
    {
        VkDeviceSize bufferSize = sizeof(Vertex2D) * ui->vertexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, ui->getVertices(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ui->getPointBuffer().vertBuffer, ui->getPointBuffer().vertHandler);

        //vertexBuffer配列にコピーしていく(vector型)
        copyBuffer(stagingBuffer, ui->getPointBuffer().vertBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //コライダー用の頂点バッファーを作成
    void VulkanBase::createVertexBuffer(std::shared_ptr<Colider> colider)
    {
        VkDeviceSize bufferSize = sizeof(*colider->getColiderOriginalVertices()) * colider->getColiderVerticesSize();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, colider->getColiderOriginalVertices(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colider->getPointBufferData()->vertBuffer, colider->getPointBufferData()->vertHandler);

        //vertexBuffer配列にコピーしていく(vector型)
        copyBuffer(stagingBuffer, colider->getPointBufferData()->vertBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //Modelクラス用のインデックスバッファーを作成する
    void VulkanBase::createIndexBuffer(GltfNode* node, std::shared_ptr<Model> model)
    {
        for(auto mesh : node->meshArray)
        {
            VkDeviceSize bufferSize = sizeof(uint32_t) * mesh->indices.size();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, mesh->indices.data(), (size_t)bufferSize);
            vkUnmapMemory(device, stagingBufferMemory);

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                model->getPointBufferData()[mesh->meshIndex].indeBuffer, model->getPointBufferData()[mesh->meshIndex].indeHandler);

            copyBuffer(stagingBuffer, model->getPointBufferData()[mesh->meshIndex].indeBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        for (size_t i = 0; i < node->children.size(); i++)
        {
            createIndexBuffer(node->children[i],model);
        }
    }

    //UI用のインデックスバッファーの作成
    void VulkanBase::createIndexBuffer(std::shared_ptr<UI> ui) 
    {
        VkDeviceSize bufferSize = sizeof(uint32_t) * ui->indexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, ui->getIndices(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ui->getPointBuffer().indeBuffer,ui->getPointBuffer().indeHandler);

        copyBuffer(stagingBuffer, ui->getPointBuffer().indeBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //コライダー用のインデックスバッファーを作成
    void VulkanBase::createIndexBuffer(std::shared_ptr<Colider> colider)
    {
        VkDeviceSize bufferSize = sizeof(*colider->getColiderIndices()) * colider->getColiderIndicesSize();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, colider->getColiderIndices(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colider->getPointBufferData()->indeBuffer, colider->getPointBufferData()->indeHandler);

        copyBuffer(stagingBuffer, colider->getPointBufferData()->indeBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //ユニフォームバッファの作成
    void VulkanBase::createUniformBuffer(int count,MappedBuffer* mappedBuffer,size_t size)
    {
        VkDeviceSize bufferSize = size * count;

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //ModelクラスのMVP行列用のユニフォームバッファの作成
    void VulkanBase::createUniformBuffer(std::shared_ptr<Model> model)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO);

        MappedBuffer* mappedBuffer = &model->getModelViewMappedBuffer();

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //gltfモデルのノードの数だけアニメーション行列のバッファーを作成する
    void VulkanBase::createUniformBuffer(GltfNode* node, std::shared_ptr<Model> model)
    {
        for(auto mesh : node->meshArray)
        {
            VkDeviceSize bufferSize = sizeof(AnimationUBO);

            MappedBuffer* mappedBuffer = &model->getAnimationMappedBufferData()[mesh->meshIndex];

            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

            vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            createUniformBuffer(node->children[i], model);
        }
    }

    //UI用
    void VulkanBase::createUniformBuffer(std::shared_ptr<UI> ui)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO2D);

        MappedBuffer* mappedBuffer = &ui->getMappedBuffer();

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //コライダー用のMVP行列のバッファーを作成
    void VulkanBase::createUniformBuffer(std::shared_ptr<Colider> colider)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO);
        if (colider->isConvex)
        {
            bufferSize += sizeof(AnimationUBO);
        }

        MappedBuffer* mappedBuffer = colider->getMappedBufferData();

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //マテリアル用のバッファーの作成
    void VulkanBase::createUniformBuffer(std::shared_ptr<Material> material)
    {
        VkDeviceSize bufferSize = sizeof(ShaderMaterial);

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            , material->sMaterialMappedBuffer.uniformBuffer, material->sMaterialMappedBuffer.uniformBufferMemory);

        vkMapMemory(device, material->sMaterialMappedBuffer.uniformBufferMemory, 0, bufferSize, 0, &material->sMaterialMappedBuffer.uniformBufferMapped);
    }

    //MVP行列とアニメーション行列のバッファーの作成 付随するコライダーの場合はそれも作成
    void VulkanBase::createUniformBuffers(std::shared_ptr<Model> model)
    {
        createUniformBuffer(model);

        createUniformBuffer(model->getRootNode(), model);

        if (model->hasColider())
        {
            createUniformBuffer(model->getColider());
        }
    }

    //ポイントライトの行列の更新
    void VulkanBase::updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights,MappedBuffer& mappedBuffer)
    {
        if (pointLights.size() == 0)
        {
            return;
        }

        PointLightUBO ubo{};

        int loopLimit = static_cast<int>(std::min(pointLights.size(), ubo.pos.size()));

        ubo.lightCount = loopLimit;

        for (int i = 0; i < loopLimit; i++)
        {
            ubo.pos[i] = glm::vec4(pointLights[i]->getPosition(),1.0f);
            ubo.color[i] = glm::vec4(pointLights[i]->color,1.0f);
        }

        memcpy(mappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
    }

    //平行光源の行列の更新
    void VulkanBase::updateUniformBuffer(std::vector<std::shared_ptr<DirectionalLight>>& directionalLights, MappedBuffer& mappedBuffer)
    {
        if (directionalLights.size() == 0)
        {
            return;
        }

        DirectionalLightUBO ubo{};
        
        int loopLimit = static_cast<int>(std::min(directionalLights.size(), ubo.dir.size()));

        ubo.lightCount = loopLimit;
        
        for (int i = 0; i < loopLimit; i++)
        {
            ubo.dir[i] = glm::vec4(directionalLights[i]->direction ,0.0f);
            ubo.color[i] = glm::vec4(directionalLights[i]->color,0.0f);
        }

        memcpy(mappedBuffer.uniformBufferMapped, &ubo, sizeof(DirectionalLightUBO));
    }

    //Modelクラスのアニメーション行列の更新をする
    void VulkanBase::updateUniformBuffer(GltfNode* node, std::shared_ptr<Model> model)
    {
        for(auto mesh : node->meshArray)
        {
            std::shared_ptr<Camera> camera = Storage::GetInstance()->accessCamera();

            std::array<glm::mat4, 128> array;
            std::fill(array.begin(), array.end(), glm::mat4(1.0f));

            AnimationUBO ubo;

            ubo.matrix = node->matrix;
            if (node->skin && node->globalHasSkinNodeIndex > -1)
            {
                ubo.boneMatrix = model->getJointMatrices(node->globalHasSkinNodeIndex);
            }
            else
            {
                ubo.boneMatrix = array;
            }

            ubo.boneCount = node->getJointCount();

            memcpy(model->getAnimationMappedBufferData()[mesh->meshIndex].uniformBufferMapped, &ubo, sizeof(ubo));

        }

        for (int i = 0; i < node->children.size(); i++)
        {
            updateUniformBuffer(node->children[i], model);
        }
    }

    //シャドウマップ作成用の行列の更新
    void VulkanBase::updateUniformBuffer(std::vector<std::shared_ptr<PointLight>>& pointLights, std::vector<std::shared_ptr<DirectionalLight>>& directionalLights)
    {
        Storage* storage = Storage::GetInstance();

        std::shared_ptr<Camera> camera = storage->accessCamera();

        glm::vec3 min, max;
        storage->calcSceneBoundingBox(min, max);

        for (int i = 0; i < directionalLights.size(); i++)
        {
            shadowMapData.matUBOs[i].view = glm::lookAt(directionalLights[i]->getPosition(), directionalLights[i]->getPosition() + directionalLights[i]->direction, glm::vec3(0.0f, 1.0f, 0.0f));
            shadowMapData.matUBOs[i].proj = shadowMapData.proj;

            memcpy(shadowMapData.mappedBuffers[i].uniformBufferMapped, &shadowMapData.matUBOs[i], sizeof(ShadowMapUBO));
        }
    }

    //ModelクラスのMVP行列の更新
    void VulkanBase::updateUniformBuffers(std::shared_ptr<Model> model)
    {
        Storage* storage = Storage::GetInstance();
        std::shared_ptr<Camera> camera = storage->accessCamera();

        MatricesUBO ubo{};

        if (model->applyScaleUV())
        {
            //モデルのスケールに合わせてuv座標を調整する
            ubo.scale = model->scale;
        }
        else
        {
            ubo.scale = glm::vec3(1.0f);
        }
        ubo.model = model->getTransformMatrix();
        ubo.view = camera->viewMat;
        ubo.proj = camera->perspectiveMat;
        ubo.worldCameraPos = glm::vec4(camera->getPosition(), 1.0f);

        ubo.lightCount = storage->getLightCount();

        std::vector<std::shared_ptr<DirectionalLight>> dLights = storage->getDirectionalLights();
        std::vector<std::shared_ptr<PointLight>> pLights = storage->getPointLights();
        for (int i = 0; i < dLights.size(); i++)
        {
            float zFar, zNear;
            camera->getzNearFar(zNear, zFar);

            ubo.lightMVP[i] = shadowMapData.proj
                * glm::lookAt(dLights[i]->getPosition(), dLights[i]->getPosition() + dLights[i]->direction, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        memcpy(model->getModelViewMappedBuffer().uniformBufferMapped, &ubo, sizeof(ubo));

        updateUniformBuffer(model->getRootNode(), model);

        if (model->hasColider())
        {
            std::shared_ptr<Camera> camera = storage->accessCamera();
            std::shared_ptr<Colider> colider = model->getColider();

            MatricesUBO ubo{};

            ubo.model = model->getTransformMatrix();
            ubo.view = camera->viewMat;
            ubo.proj = camera->perspectiveMat;
            ubo.worldCameraPos = glm::vec4(camera->getPosition(), 1.0);

            memcpy(colider->getMappedBufferData()->uniformBufferMapped, &ubo, sizeof(ubo));
        }
    }

    void VulkanBase::updateUniformBuffer(std::shared_ptr<UI> ui)
    {
        MatricesUBO2D ubo{};
        ubo.transformMatrix = ui->getTransfromMatrix();
        ubo.projection = ui->getProjectMatrix();

        memcpy(ui->getMappedBuffer().uniformBufferMapped, &ubo, sizeof(ubo));
    }

    //キューブマップ描画用の行列の更新
    void VulkanBase::updateUniformBuffers_Cubemap(std::shared_ptr<Model> cubemap)
    {
        Storage* storage = Storage::GetInstance();
        std::shared_ptr<Camera> camera = storage->accessCamera();

        MatricesUBO ubo{};

        ubo.view = camera->cubemapViewMat;
        ubo.proj = camera->perspectiveMat;

        memcpy(cubemap->getModelViewMappedBuffer().uniformBufferMapped, &ubo, sizeof(ubo));
    }

    void VulkanBase::updateColiderVertices_OnlyDebug(std::shared_ptr<Colider> colider)
    {
        VkDeviceSize bufferSize = sizeof(*colider->getColiderOriginalVertices()) * colider->getColiderVerticesSize();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, colider->getColiderOriginalVertices(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        //vertexBuffer配列にコピーしていく(vector型)
        copyBuffer(stagingBuffer, colider->getPointBufferData()->vertBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //descriptorPoolの作成 descriptorSetはここから作成
    void VulkanBase::createDescriptorPool()
    {
        std::array<VkDescriptorPoolSize,2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(100 * swapChainImages.size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(100 * swapChainImages.size());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_VKDESCRIPTORSET);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    //descriptorSetのバッファーの用意
    //gltfモデル用
    void VulkanBase::allocateDescriptorSet(VkDescriptorSetLayout& layout,GltfNode * node, std::shared_ptr<Model> model)
    {
        for(auto mesh : node->meshArray)
        {
            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool;
                allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
                allocInfo.pSetLayouts = &layout;

                VkDescriptorSet descriptorSet;

                if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to allocate descriptor sets!");
                }

                if (descriptorSetCount > MAX_VKDESCRIPTORSET)
                {
                    throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
                }
                descriptorSetCount++;

                model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet = descriptorSet;
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            allocateDescriptorSet(layout,node->children[i], model);
        }
    }

    //コライダー用
    void VulkanBase::allocateDescriptorSet(std::shared_ptr<Material> material)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
        allocInfo.pSetLayouts = &modelDescriptor.materialLayout;

        VkDescriptorSet descriptorSet;

        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        if (descriptorSetCount > 100)
        {
            throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
        }
        descriptorSetCount++;

        material->descriptorSet = descriptorSet;
    }

    //ShaderMaterial用
    void VulkanBase::allocateDescriptorSet(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet descriptorSet;

        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        if (descriptorSetCount > MAX_VKDESCRIPTORSET)
        {
            throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
        }
        descriptorSetCount++;

        model->getColider()->setDescriptorSet(descriptorSet);
    }

    //gltfモデル用
    void VulkanBase::allocateDescriptorSets(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model)
    {
        allocateDescriptorSet(layout,model->getRootNode(), model);
        if (model->hasColider())
        {
            allocateDescriptorSet(layout,model);
        }
    }

    //descriptorSetの作成
    //gltfモデルの作成
    void VulkanBase::createDescriptorSet(GltfNode* node,std::shared_ptr<Model> model)
    {
        for(auto mesh : node->meshArray)
        {
            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = model->getModelViewMappedBuffer().uniformBuffer;
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(MatricesUBO);

                VkDescriptorBufferInfo animationBufferInfo{};
                animationBufferInfo.buffer = model->getAnimationMappedBufferData()[i].uniformBuffer;
                animationBufferInfo.offset = 0;
                animationBufferInfo.range = sizeof(AnimationUBO);

                std::vector<VkWriteDescriptorSet> descriptorWrites;
                descriptorWrites.resize(2);

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet;
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet;
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pBufferInfo = &animationBufferInfo;

                vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            createDescriptorSet(node->children[i], model);
        }
    }

    //コライダー用
    void VulkanBase::createDescriptorSet(std::shared_ptr<Model> model)
    {
        std::shared_ptr<Colider> colider = model->getColider();

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = colider->getMappedBufferData()->uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatricesUBO);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = colider->getDescSetData().descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    //マテリアル用
    void VulkanBase::createDescriptorSet(std::shared_ptr<Material> material, std::shared_ptr<GltfModel> gltfModel)
    {
        std::vector<VkDescriptorImageInfo> imageInfo(MAX_TEXTURE_COUNT);

        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = emptyImage.emptyTex->view;
        info.sampler = emptyImage.emptyTex->sampler;
        std::fill(imageInfo.begin(), imageInfo.end(), info);

        if (material->baseColorTextureIndex != -1)
        {
            imageInfo[0].imageView = gltfModel->textureDatas[material->baseColorTextureIndex]->view;
            imageInfo[0].sampler = gltfModel->textureDatas[material->baseColorTextureIndex]->sampler;
        }
        if (material->metallicRoughnessTextureIndex != -1)
        {
            imageInfo[1].imageView = gltfModel->textureDatas[material->metallicRoughnessTextureIndex]->view;
            imageInfo[1].sampler = gltfModel->textureDatas[material->metallicRoughnessTextureIndex]->sampler;
        }
        if (material->normalTextureIndex != -1)
        {
            imageInfo[2].imageView = gltfModel->textureDatas[material->normalTextureIndex]->view;
            imageInfo[2].sampler = gltfModel->textureDatas[material->normalTextureIndex]->sampler;
        }
        if (material->occlusionTextureIndex != -1)
        {
            imageInfo[3].imageView = gltfModel->textureDatas[material->occlusionTextureIndex]->view;
            imageInfo[3].sampler = gltfModel->textureDatas[material->occlusionTextureIndex]->sampler;
        }
        if (material->emissiveTextureIndex != -1)
        {
            imageInfo[4].imageView = gltfModel->textureDatas[material->emissiveTextureIndex]->view;
            imageInfo[4].sampler = gltfModel->textureDatas[material->emissiveTextureIndex]->sampler;
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = material->sMaterialMappedBuffer.uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ShaderMaterial);

        std::vector<VkWriteDescriptorSet> descriptorWrites(MAX_TEXTURE_COUNT + 1);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = material->descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        for (int i = 1; i < descriptorWrites.size(); i++)
        {
            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = material->descriptorSet;
            descriptorWrites[i].dstBinding = static_cast<uint32_t>(i);
            descriptorWrites[i].dstArrayElement = 0;
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[i].descriptorCount = 1;
            descriptorWrites[i].pImageInfo = &imageInfo[i - 1];
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    //gltfモデルの作成
    void VulkanBase::createDescriptorSets(std::shared_ptr<Model> model)
    {
        createDescriptorSet(model->getRootNode(),model);
        if (model->hasColider())
        {
            createDescriptorSet(model);
        }
    }

    //キューブマップ用
    void VulkanBase::createDescriptorSet_CubeMap(GltfNode* node, std::shared_ptr<Model> model)
    {
        for(auto mesh : node->meshArray)
        {
            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = model->getModelViewMappedBuffer().uniformBuffer;
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(MatricesUBO);

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet;
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, static_cast<uint32_t>(1), &descriptorWrite, 0, nullptr);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            createDescriptorSet_CubeMap(node->children[i], model);
        }
    }

    //バッファの作成
    void VulkanBase::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
           throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    //使い捨てのコマンドの記録開始
    VkCommandBuffer VulkanBase::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    //コマンドの記録終了
    void VulkanBase::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        auto a = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

        if (a != VK_SUCCESS)
        {
            throw std::runtime_error("queue submit error");
        }

        auto b = vkQueueWaitIdle(graphicsQueue);
        if (b != VK_SUCCESS)
        {
            throw std::runtime_error("wait idle failed");
        }

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    //バッファ上のデータのコピー
    void VulkanBase::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    uint32_t VulkanBase::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    //コマンドバッファーを二つ作成する
    void VulkanBase::createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    //UIのレンダリング
    void VulkanBase::drawUI(bool beginRenderPass, VkCommandBuffer& commandBuffer,uint32_t imageIndex)
    {
        if (beginRenderPass)
        {
            vkResetCommandBuffer(commandBuffers[currentFrame], 0);

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;

            VkClearValue clearValue{};
            clearValue.depthStencil = { 1.0f,0 };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearValue;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiRender.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        Storage* storage = Storage::GetInstance();

        for (int i = 0; i < storage->getUI().size(); i++)
        {
            std::shared_ptr<UI> ui = storage->getUI()[i];

            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &ui->getPointBuffer().vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffer, ui->getPointBuffer().indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                uiRender.pLayout, 0, 1, &ui->getDescriptorSet(), 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, ui->indexCount, 1, 0, 0, 0);
        }

        if (beginRenderPass)
        {
            vkCmdEndRenderPass(commandBuffer);

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    //指定されたUIのみレンダリング ロード画面の描画を想定
    void VulkanBase::drawUI(std::shared_ptr<UI> ui, bool beginRenderPass, VkCommandBuffer& commandBuffer,uint32_t imageIndex)
    {
        if (beginRenderPass)
        {
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;

            std::vector<VkClearValue> clearValues(2);
            clearValues[0].color = { { 1.0f, 1.0f, 1.0f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f,0 };
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiRender.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &ui->getPointBuffer().vertBuffer, offsets);

        vkCmdBindIndexBuffer(commandBuffer, ui->getPointBuffer().indeBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            uiRender.pLayout, 0, 1, &ui->getDescriptorSet(), 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, ui->indexCount, 1, 0, 0, 0);

        if (beginRenderPass)
        {
            vkCmdEndRenderPass(commandBuffer);

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    //ロードUIの描画
    void VulkanBase::drawLoading()
    {
        std::shared_ptr<UI> loadUI = Storage::GetInstance()->getLoadUI();

        vkWaitForFences(device, 1, &multiThreadFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &multiThreadFences[currentFrame]);

        vkResetCommandBuffer(uiRender.loadCommandBuffers[currentFrame], 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(uiRender.loadCommandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        //一応ロードUIの行列を更新する
        updateUniformBuffer(loadUI);

        //ここでロードUIの描画
        drawUI(loadUI, true, uiRender.loadCommandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &uiRender.loadCommandBuffers[currentFrame];

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

        if (vkQueueSubmit(multiThreadGraphicQueue, 1, &submitInfo, multiThreadFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

        std::vector<VkSwapchainKHR> swapChains = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains.data();

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(multiThreadPresentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    //ロード画面の描画の終了
    void VulkanBase::stopLoading()
    {
        //すべてのフェンスから実行完了の信号を受け取る
        vkWaitForFences(device, static_cast<uint32_t>(multiThreadFences.size()), multiThreadFences.data()
            , VK_TRUE, UINT64_MAX);
        //すべてのフェンスをリセット
        vkResetFences(device, static_cast<uint32_t>(multiThreadFences.size()), multiThreadFences.data());

        //キュー上の処理がすべて終わるまで待つ
        vkQueueWaitIdle(multiThreadGraphicQueue);
        vkQueueWaitIdle(multiThreadPresentQueue);
    }

    //キューブマップのレンダリング
    void VulkanBase::drawCubeMap(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer)
    {
        Storage* storage = Storage::GetInstance();

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, node->descriptorInfo.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        for(auto mesh : node->meshArray)
        {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffer, model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                std::vector<VkDescriptorSet> descriptorSets =
                {
                    model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet,
                    cubemapData.descriptorSet
                };

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    node->descriptorInfo.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            drawCubeMap(node->children[i], model, commandBuffer);
        }
    }

    //gltfモデルの描画
    void VulkanBase::drawMesh(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer)
    {
        Storage* storage = Storage::GetInstance();

        PushConstantObj constant = { node->getMatrix() };

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, node->descriptorInfo.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        for(auto mesh : node->meshArray)
        {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffer, model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (int i = 0;i < mesh->primitives.size();i++)
            {
                std::shared_ptr<Material> material = model->getGltfModel()->materials[mesh->primitives[i].materialIndex];

                std::vector<VkDescriptorSet> descriptorSets =
                {
                    model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet,//MVP行列とアニメーション行列
                    material->descriptorSet,//マテリアル
                    storage->getPointLightDescriptorSet(),//ポイントライト
                    storage->getDirectionalLightDescriptorSet(),//平行光源
                    shadowMapData.descriptorSets[0],//シャドウマップ
                    iblDiffuse.descriptorSet,//IBLのdiffuse
                    iblSpecularReflection.descriptorSet,//IBLのspecularの鏡面反射
                    iblSpecularBRDF.descriptorSet//IBLのspecularのBRDF
                };

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    node->descriptorInfo.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdPushConstants(commandBuffer, node->descriptorInfo.pLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObj), &constant);

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            drawMesh(node->children[i], model,commandBuffer);
        }
    }

    //シャドウマップの作成
    void VulkanBase::calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer,OffScreenPass& pass)
    {
        Storage* storage = Storage::GetInstance();

        for(auto mesh : node->meshArray)
        {
            PushConstantObj constant = { node->getMatrix() };

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)pass.width;
            viewport.height = (float)pass.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = { (uint32_t)pass.width,(uint32_t)pass.height};
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffer, model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            std::vector<VkDescriptorSet> descriptorSets(2);
            descriptorSets[0] = pass.descriptorSets[0];

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                descriptorSets[1] = model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet;

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pass.pipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdPushConstants(commandBuffer, pass.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObj), &constant);

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            calcDepth(node->children[i], model, commandBuffer,pass);
        }
    }

    //シャドウマップの用意
    void VulkanBase::setupShadowMapDepth(VkCommandBuffer& commandBuffer)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = shadowMapData.passData.renderPass;
        renderPassInfo.framebuffer = shadowMapData.passData.frameBuffer[0];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent.width = shadowMapData.passData.width;
        renderPassInfo.renderArea.extent.height = shadowMapData.passData.height;

        VkClearValue clearValue{};
        clearValue.depthStencil = { 1.0f,0 };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //1.75f, 0.0f, 5.75f
        vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);

        Storage* storage = Storage::GetInstance();
        for (auto& model : storage->getModels())
        {
            calcDepth(model->getRootNode(), model, commandBuffer,shadowMapData.passData);
        }

        vkCmdEndRenderPass(commandBuffer);
    }

    //通常のレンダリングのコマンドを記録していく
    void VulkanBase::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        setupShadowMapDepth(commandBuffer);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2>clearValues{};
        clearValues[0].color = { {0.0f,0.0f,0.0f,1.0f} };
        clearValues[1].depthStencil = { 1.0f,0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //UIの描画
        drawUI(false,commandBuffer,imageIndex);

        Storage* storage = Storage::GetInstance();
        
        for (auto model:storage->getModels())
        {
            drawMesh((model)->getRootNode(),model,commandBuffer);

            if ((model)->hasColider() && coliderDraw)
            {
                std::shared_ptr<Colider> colider = (model)->getColider();

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelDescriptor.coliderPipeline);

                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float)swapChainExtent.width;
                viewport.height = (float)swapChainExtent.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                VkRect2D scissor{};
                scissor.offset = { 0, 0 };
                scissor.extent = swapChainExtent;
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                VkDeviceSize offsets[] = { 0 };

                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &colider->getPointBufferData()->vertBuffer, offsets);

                vkCmdBindIndexBuffer(commandBuffer, colider->getPointBufferData()->indeBuffer, 0, VK_INDEX_TYPE_UINT32);

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    modelDescriptor.coliderPipelineLayout, 0, 1, &colider->getDescSetData().descriptorSet, 0, nullptr);

                vkCmdDrawIndexed(commandBuffer, colider->getColiderIndicesSize(), 1, 0, 0, 0);
            }
        }

        drawCubeMap(storage->getCubeMap()->getRootNode(), storage->getCubeMap(), commandBuffer);//キューブマッピングの描画

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void VulkanBase::createSyncObjects() {//描画の動機用の変数を用意
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        multiThreadFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS ||
                vkCreateFence(device,&fenceInfo,nullptr,&multiThreadFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    //通常のレンダリングを開始する
    void VulkanBase::drawFrame()
    {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        Storage* storage = Storage::GetInstance();

        for (auto model:storage->getModels())
        {
            updateUniformBuffers(model);//3Dモデルの座標変換行列などを更新
        }
        for (auto ui : storage->getUI())
        {
            updateUniformBuffer(ui);//UIについての座標変換行列やテクスチャを更新
        }

        updateUniformBuffer(storage->getPointLights(), storage->getPointLightsBuffer());//ポイントライトの座標、色の更新
        updateUniformBuffer(storage->getDirectionalLights(), storage->getDirectionalLightsBuffer());//ディレクショナルライトの向き、色の更新
        updateUniformBuffer(storage->getPointLights(), storage->getDirectionalLights());//シャドウマップ用の更新

        updateUniformBuffers_Cubemap(storage->getCubeMap());

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::vector<VkSemaphore> waitSemaphores = { imageAvailableSemaphores[currentFrame] };
        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        std::vector<VkSemaphore> signalSemaphores = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores.data();

        std::vector<VkSwapchainKHR> swapChains = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains.data();

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    //シェーダの作成
    VkShaderModule VulkanBase::createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkSurfaceFormatKHR VulkanBase::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VulkanBase::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanBase::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

    SwapChainSupportDetails VulkanBase::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool VulkanBase::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices VulkanBase::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> VulkanBase::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool VulkanBase::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    //gltfモデルの頂点バッファーなどの作成、付随するコライダーの頂点のバッファーも用意
    void VulkanBase::createMeshesData(std::shared_ptr<Model> model)
    {
        createVertexBuffer(model->getRootNode(),model);
        createIndexBuffer(model->getRootNode(), model);

        if (model->hasColider())
        {
            createVertexBuffer(model->getColider());
            createIndexBuffer(model->getColider());
        }
    }

    //gltfモデルの各ノードにパイプラインとそのレイアウトを設定する
    void VulkanBase::createDescriptorInfo(VkPipelineLayout& pLayout, VkPipeline& pipeline, GltfNode* node, std::shared_ptr<Model> model)
    {
        node->descriptorInfo.pLayout = pLayout;
        node->descriptorInfo.pipeline = pipeline;

        for (int i = 0; i < node->children.size(); i++)
        {
            createDescriptorInfo(pLayout,pipeline,node->children[i], model);
        }
    }

    //コライダーにパイプラインとそのレイアウトを設定する
    void VulkanBase::createDescriptorInfo(std::shared_ptr<Colider> colider)
    {
        DescriptorInfo& info = colider->getDescInfo();

        info.pLayout = modelDescriptor.coliderPipelineLayout;
        info.pipeline = modelDescriptor.coliderPipeline;
    }

    //gltfモデルの各ノードにパイプラインとそのレイアウトを設定する
    void VulkanBase::createDescriptorInfos(VkPipelineLayout& pLayout,VkPipeline& pipeline,std::shared_ptr<Model> model)
    {
        if (!model->getGltfModel()->setup)
        {
            createDescriptorInfo(pLayout,pipeline,model->getRootNode(), model);
        }

        if (model->hasColider())
        {
            createDescriptorInfo(model->getColider());
        }
    }

    //ShaderMaterialの初期設定とバッファーの用意
    void VulkanBase::createShaderMaterialUBO(std::shared_ptr<Material> material)
    {
        material->setupShaderMaterial();

        createUniformBuffer(material);

        memcpy(material->sMaterialMappedBuffer.uniformBufferMapped, &material->shaderMaterial, sizeof(ShaderMaterial));
    }

    //ダミーテクスチャのデータの作成
    void VulkanBase::createEmptyImage()
    {
        emptyImage.emptyTex = new TextureData();

        createTextureImage();
        createTextureImageView();
        createTextureSampler();

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &samplerLayoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &emptyImage.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
        allocInfo.pSetLayouts = &emptyImage.layout;

        VkDescriptorSet descriptorSet;

        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        if (descriptorSetCount > MAX_VKDESCRIPTORSET)
        {
            throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
        }
        descriptorSetCount++;

        emptyImage.descriptorSet = descriptorSet;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = emptyImage.emptyTex->view;
        imageInfo.sampler = emptyImage.emptyTex->sampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = emptyImage.descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanBase::prepareDescriptorSets()//通常のレンダリングで必要なDescriptorSetのレイアウトを作成
    {
        //まずVkDescriptorSetLayoutをあらかじめ作っておく
        {
            //3Dモデルの座標変換用のレイアウト
            std::vector<VkDescriptorSetLayoutBinding> bindings(2);
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding uboLayoutBindingAnimation{};
            uboLayoutBindingAnimation.binding = 1;
            uboLayoutBindingAnimation.descriptorCount = 1;
            uboLayoutBindingAnimation.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBindingAnimation.pImmutableSamplers = nullptr;
            uboLayoutBindingAnimation.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            bindings[0] = uboLayoutBinding;
            bindings[1] = uboLayoutBindingAnimation;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &modelDescriptor.layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        {
            //マテリアル用のレイアウト
            std::vector<VkDescriptorSetLayoutBinding> bindings(MAX_TEXTURE_COUNT + 1);
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[0] = uboLayoutBinding;

            for (int i = 1; i < MAX_TEXTURE_COUNT + 1; i++)
            {
                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = i;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                bindings[i] = samplerLayoutBinding;
            }

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount =static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &modelDescriptor.materialLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        {
            //ライト用のレイアウト
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &uboLayoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &modelDescriptor.lightLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        {
            //IBL用のレイアウト
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 0;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &samplerLayoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblDiffuse.mainPassLayout) != VK_SUCCESS)//IBLのdiffuse
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblSpecularReflection.mainPassLayout) != VK_SUCCESS)//IBLのspecularの鏡面反射用
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblSpecularBRDF.mainPassLayout) != VK_SUCCESS)//IBLのspecularのBRDF用
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
    }

    //ライトのdescriptorSet関係のデータを作成
    void VulkanBase::createDescriptorData(MappedBuffer& mappedBuffer,VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet, unsigned long long size,VkShaderStageFlags frag)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
        allocInfo.pSetLayouts = &layout;

        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        if (descriptorSetCount > MAX_VKDESCRIPTORSET)
        {
            throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
        }
        descriptorSetCount++;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mappedBuffer.uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = size;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    //シャドウマップ作成用のdescriptorSet関係のデータの作成
    void VulkanBase::createDescriptorData_ShadowMap(std::vector<VkDescriptorSet>& descriptorSets,OffScreenPass& pass,VkDescriptorSetLayout& layout)
    {
        //レイアウトはpreapreDescriptorにて作成済み
        //デプスバッファを入力とするdescriptorsetのレイアウトの作成
        {
            //シャドウマップ用
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 0;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &samplerLayoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &shadowMapData.layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
        allocInfo.pSetLayouts = &layout;

        for (int i = 0;i < descriptorSets.size();i++)
        {
            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            imageInfo.imageView = pass.imageAttachment[i].view;
            imageInfo.sampler = pass.sampler;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite,0,nullptr);
        }
    }

    //ポイントライトのgpu上のバッファーなどを作成
    void VulkanBase::setPointLights(std::vector<std::shared_ptr<PointLight>> lights)
    {
        Storage* storage = Storage::GetInstance();

        createUniformBuffer(1, &storage->getPointLightsBuffer(), sizeof(PointLightUBO));

        createDescriptorData(storage->getPointLightsBuffer(), modelDescriptor.lightLayout,
            storage->getPointLightDescriptorSet(), sizeof(PointLightUBO), VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    //平行光源のgpu上のバッファーなどを作成
    void VulkanBase::setDirectionalLights(std::vector<std::shared_ptr<DirectionalLight>> lights)
    {
        Storage* storage = Storage::GetInstance();

        createUniformBuffer(1, &storage->getDirectionalLightsBuffer(), sizeof(DirectionalLightUBO));

        createDescriptorData(storage->getDirectionalLightsBuffer(), modelDescriptor.lightLayout,
            storage->getDirectionalLightDescriptorSet(), sizeof(DirectionalLightUBO), VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    //gltfモデルの持つマテリアル用のデータの作成
    void VulkanBase::setGltfModelData(std::shared_ptr<GltfModel> gltfModel)
    {
        /*テクスチャ関連の設定を持たせる*/
        createTextureImage(gltfModel, VK_FORMAT_R8G8B8A8_UNORM);
        createTextureImageView(gltfModel, VK_FORMAT_R8G8B8A8_UNORM);
        createTextureSampler(gltfModel);

        for (std::shared_ptr<Material> material : gltfModel->materials)
        {
            createShaderMaterialUBO(material);
            /*ここからパイプラインは、同じグループのモデルでは使いまわせる*/
            /*ディスクリプタセットは、テクスチャデータが異なる場合は使いまわせない*/

            /*ディスクリプタ用のメモリを空ける*/
            allocateDescriptorSet(material);//マテリアルが複数ある場合エラー

            /*ディスクリプタセットを作る*/
            createDescriptorSet(material, gltfModel);
        }
    }

    //シャドウマップ用ンデータを用意する、引数としてシーン上のライトの数だけオフスクリーンレンダリングを行う
    void VulkanBase::prepareShadowMapping(int lightCount)
    {
        shadowMapData.setFrameCount(lightCount);
        shadowMapData.descriptorSets.resize(1);
        shadowMapData.shadowMapScale = 4;

        shadowMapData.proj = glm::ortho(shadowMapLeft, shadowMapRight, shadowMapBottom, shadowMapTop, shadowMapNear, shadowMapFar);

        shadowMapData.passData.width = swapChainExtent.width * shadowMapData.shadowMapScale;
        shadowMapData.passData.height = swapChainExtent.height * shadowMapData.shadowMapScale;

        for (auto& attachment : shadowMapData.passData.imageAttachment)
        {
            createImage(shadowMapData.passData.width, shadowMapData.passData.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D16_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.memory);
            attachment.view = createImageView(attachment.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT, 1,1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, shadowMapData.passData.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_D16_UNORM;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// レンダーパス開始時に深度をクリア
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// 深度を読み取るため、深度アタッチメントの結果を保存することが重要
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// アタッチメントの初期レイアウトは気にしない
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// レンダーパス終了時に、アタッチメントはシェーダー読み取り用に遷移される

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 0;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// レンダーパス中に、アタッチメントは深度/ステンシルとして使用される

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;													// カラーアタッチメントなし
        subpass.pDepthStencilAttachment = &depthReference;									// 深度アタッチメントへの参照

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//最初に
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//次
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &shadowMapData.passData.renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create renderpass");
        }

        // Create frame buffer
        for (int i = 0; i < shadowMapData.passData.frameBuffer.size(); i++)
        {
            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = shadowMapData.passData.renderPass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &shadowMapData.passData.imageAttachment[i].view;
            fbufCreateInfo.width = shadowMapData.passData.width;
            fbufCreateInfo.height = shadowMapData.passData.height;
            fbufCreateInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &shadowMapData.passData.frameBuffer[i]))
            {
                throw std::runtime_error("failed create frame buffer");
            }
        }

        for (int i = 0; i < lightCount; i++)
        {
            createUniformBuffer(1, &shadowMapData.mappedBuffers[i], sizeof(ShadowMapUBO));
        }

        VkDescriptorSetLayoutBinding layoutBinding;
        layoutBinding.binding = 0;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &layoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &shadowMapData.passData.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        for (int i = 0; i < lightCount; i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &shadowMapData.passData.layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &shadowMapData.passData.descriptorSets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = shadowMapData.mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(ShadowMapUBO);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = shadowMapData.passData.descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(1), &descriptorWrite, 0, nullptr);
        }

        createShadowMapPipeline("shaders/shadowMapping.vert.spv"
            , shadowMapData.passData.layout, shadowMapData.passData.pipelineLayout, shadowMapData.passData.pipeline,shadowMapData.passData.renderPass);

        createDescriptorData_ShadowMap(shadowMapData.descriptorSets,shadowMapData.passData,shadowMapData.layout);//デプスバッファをテクスチャとして利用するためのdescriptorSet
    }

    //UI描画用のパイプラインなどを用意
    void VulkanBase::prepareUIRendering()
    {
        uiRender.vertPath = "shaders/ui.vert.spv";
        uiRender.fragPath = "shaders/ui.frag.spv";

        //VkDescriptorSetLayoutの作成
        {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
            layoutBindings[0].binding = 0;
            layoutBindings[0].descriptorCount = 1;
            layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBindings[0].pImmutableSamplers = nullptr;
            layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            layoutBindings[1].binding = 1;
            layoutBindings[1].descriptorCount = 1;
            layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBindings[1].pImmutableSamplers = nullptr;
            layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
            layoutInfo.pBindings = layoutBindings.data();

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &uiRender.layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        //パイプラインレイアウトの作成
        {
            auto vertShaderCode = readFile(uiRender.vertPath);
            auto fragShaderCode = readFile(uiRender.fragPath);

            VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
            bindingDescription.stride = sizeof(Vertex2D);

            attributeDescriptions.resize(2);
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex2D, uv);

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_NONE;//カリングを無効
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_TRUE;
            multisampling.minSampleShading = 0.2f;
            multisampling.rasterizationSamples = msaaSamples;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.minDepthBounds = 0.0f;
            depthStencil.maxDepthBounds = 1.0f;
            depthStencil.stencilTestEnable = VK_FALSE;
            depthStencil.front = {};
            depthStencil.back = {};

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &uiRender.layout;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &uiRender.pLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = uiRender.pLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDepthStencilState = &depthStencil;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &uiRender.pipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            //パイプラインの作成

            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
        }

        //ロードUIの表示用のコマンドバッファ
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = multiThreadCommandPool;
            allocInfo.commandBufferCount = static_cast<uint32_t>(uiRender.loadCommandBuffers.size());

            vkAllocateCommandBuffers(device, &allocInfo, uiRender.loadCommandBuffers.data());
        }
    }

    //各種ライトのバッファなどの作成
    void VulkanBase::setLightData(std::vector<std::shared_ptr<PointLight>> pointLights, std::vector<std::shared_ptr<DirectionalLight>> dirLights)
    {
        setPointLights(pointLights);
        setDirectionalLights(dirLights);
    }

    //各種バッファーなどのデータの作成 lua実行後に行う
    void VulkanBase::prepareDescriptorData(int lightCount)
    {
        prepareShadowMapping(lightCount);//シャドウマップの用意

        /*グラフィックスパイプラインを作る*/
        createGraphicsPipeline("shaders/vert.spv", "shaders/frag.spv",
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, modelDescriptor.layout, modelDescriptor.texturePipelineLayout, modelDescriptor.texturePipeline);//普通の3Dモデル表示用

        createGraphicsPipeline("shaders/line.vert.spv", "shaders/line.frag.spv",
            VK_PRIMITIVE_TOPOLOGY_LINE_LIST, modelDescriptor.layout, modelDescriptor.coliderPipelineLayout, modelDescriptor.coliderPipeline);//コライダー表示用
    }

    //Modelクラスの持つバッファーの作成
    void VulkanBase::setModelData(std::shared_ptr<Model> model)
    {
        /*頂点、インデックスバッファーを持たせる*/
        createMeshesData(model);

        /*UnifomBufferを持たせる*/
        createUniformBuffers(model);

        /*ここからパイプラインは、同じグループのモデルでは使いまわせる*/
        /*ディスクリプタセットは、テクスチャデータが異なる場合は使いまわせない*/
        createDescriptorInfos(modelDescriptor.texturePipelineLayout, modelDescriptor.texturePipeline,model);

        /*ディスクリプタ用のメモリを空ける*/
        allocateDescriptorSets(modelDescriptor.layout,model);//マテリアルが複数ある場合エラー

        /*ディスクリプタセットを作る*/
        createDescriptorSets(model);

        model->getGltfModel()->setup = true;
    }

    //キューブマップの作成
    void VulkanBase::prepareCubemapTextures()
    {
        //hdriTextureのデータを作る
        {
            createTextureImage(cubemapData.srcHdriTexture, Storage::GetInstance()->getCubemapImage(),cubemapData.format);
            createTextureImageView(cubemapData.srcHdriTexture, cubemapData.format);
            createTextureSampler(cubemapData.srcHdriTexture);
        }

        cubemapData.setFrameCount(CUBEMAP_FACE_COUNT);

        //キューブマッピング用にレンダリングイメージを正方形にしておく
        std::shared_ptr<ImageData> cubemapImage = Storage::GetInstance()->getCubemapImage();
        int renderSize = std::min(cubemapImage->getWidth(), cubemapImage->getHeight());

        cubemapData.passData.width = renderSize;
        cubemapData.passData.height = renderSize;

        //レンダリングの結果の出力用のバッファの作成
        for (auto& attachment : cubemapData.passData.imageAttachment)
        {
            createImage(renderSize,renderSize, 1, VK_SAMPLE_COUNT_1_BIT, cubemapData.format,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.memory);
            attachment.view = createImageView(attachment.image, VK_IMAGE_VIEW_TYPE_2D, cubemapData.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, cubemapData.passData.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = cubemapData.format;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// レンダーパス開始時に深度をクリア
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// アタッチメントの情報を記録
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// アタッチメントの初期レイアウトは気にしない
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// レンダーパス終了時に、アタッチメントはシェーダー読み取り用に遷移される

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;									

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//最初に
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = 0;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//次
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &cubemapData.passData.renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create renderpass");
        }

        // Create frame buffer
        for (int i = 0; i < cubemapData.passData.frameBuffer.size(); i++)
        {
            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = cubemapData.passData.renderPass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &cubemapData.passData.imageAttachment[i].view;
            fbufCreateInfo.width = cubemapData.passData.width;
            fbufCreateInfo.height = cubemapData.passData.height;
            fbufCreateInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &cubemapData.passData.frameBuffer[i]))
            {
                throw std::runtime_error("failed create frame buffer");
            }
        }

        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            createUniformBuffer(1, &cubemapData.mappedBuffers[i], sizeof(MatricesUBO));
        }

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].pImmutableSamplers = nullptr;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[1].pImmutableSamplers = nullptr;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &cubemapData.passData.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &cubemapData.passData.layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &cubemapData.passData.descriptorSets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = cubemapData.mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = cubemapData.srcHdriTexture->view;
            info.sampler = cubemapData.srcHdriTexture->sampler;

            std::vector<VkWriteDescriptorSet> descriptorWrites(2);
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = cubemapData.passData.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = cubemapData.passData.descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &info;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        createCalcCubeMapPipeline("shaders/calcCubemap.vert.spv","shaders/calcCubemap.frag.spv"
            , cubemapData.passData.layout, cubemapData.passData.pipelineLayout, cubemapData.passData.pipeline, cubemapData.passData.renderPass);
    }

    void VulkanBase::prepareIBL(std::string vertShaderPath, std::string fragShaderPath
        ,OffScreenPass& passData,VkFormat format,uint32_t mipmapLevel,std::vector<MappedBuffer>& mappedBuffers)
    {
        //レンダリングの結果の出力用のバッファの作成
        for (auto& attachment : passData.imageAttachment)
        {
            createImage(passData.width, passData.height, mipmapLevel, VK_SAMPLE_COUNT_1_BIT, format,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.memory);

            attachment.view = createImageView(attachment.image, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT, mipmapLevel, 1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, passData.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = format;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// レンダーパス開始時に深度をクリア
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// アタッチメントの情報を記録
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// アタッチメントの初期レイアウトは気にしない
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// レンダーパス終了時に、アタッチメントはシェーダー読み取り用に遷移される

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//最初に
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = 0;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//次
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &passData.renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create renderpass");
        }

        // Create frame buffer
        for (int i = 0; i < passData.frameBuffer.size(); i++)
        {
            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = passData.renderPass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &passData.imageAttachment[i].view;
            fbufCreateInfo.width = passData.width;
            fbufCreateInfo.height = passData.height;
            fbufCreateInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &passData.frameBuffer[i]))
            {
                throw std::runtime_error("failed create frame buffer");
            }
        }

        for (int i = 0; i < static_cast<int>(mappedBuffers.size()); i++)
        {
            createUniformBuffer(1, &mappedBuffers[i], sizeof(MatricesUBO));
        }

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].pImmutableSamplers = nullptr;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[1].pImmutableSamplers = nullptr;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &passData.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        for (int i = 0; i < passData.descriptorSets.size(); i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &passData.layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &passData.descriptorSets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            //キューブマッピングで使う行列のバッファを流用する
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            //6つのレイヤーを持つ画像を参照するようにする
            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = cubemapData.multiTexture->view;
            info.sampler = cubemapData.multiTexture->sampler;

            std::vector<VkWriteDescriptorSet> descriptorWrites(2);
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = passData.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = passData.descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &info;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        //ここで指定されるシェーダはiblのテクスチャ作成用
        createCalcIBLMapPipeline(vertShaderPath, fragShaderPath, passData.layout, passData.pipelineLayout, 1,&passData.pipeline, 1,&passData.renderPass);
    }

    //specularのIBLの作成のため、レンダーパスを複数作成
    void VulkanBase::prepareIBL(IBLSpecularReflection& iblSpecular)
    {
        //レンダリングの結果の出力用のバッファの作成
        for (int i = 0; i < iblSpecular.imageAttachment.size(); i++)
        {
            createImage(iblSpecular.mipmapLevelSize[i % iblSpecular.mipmapLevel], iblSpecular.mipmapLevelSize[i % iblSpecular.mipmapLevel], 1, VK_SAMPLE_COUNT_1_BIT, iblSpecular.format,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, iblSpecular.imageAttachment[i].image, iblSpecular.imageAttachment[i].memory);
            iblSpecular.imageAttachment[i].view =
                createImageView(iblSpecular.imageAttachment[i].image, VK_IMAGE_VIEW_TYPE_2D, iblSpecular.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, iblSpecular.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = iblSpecular.format;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// レンダーパス開始時に深度をクリア
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// アタッチメントの情報を記録
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// アタッチメントの初期レイアウトは気にしない
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// レンダーパス終了時に、アタッチメントはシェーダー読み取り用に遷移される

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//最初に
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = 0;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//次
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassCreateInfo.pDependencies = dependencies.data();

        for (int i = 0; i < iblSpecular.renderPass.size(); i++)
        {
            if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &iblSpecular.renderPass[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create renderpass");
            }
        }

        //複数のレンダーパスに対して、フレームバッファの作成
        //一つの行列に対して、複数の解像度のパスが続く
        for (int i = 0; i < iblSpecular.frameBuffer.size(); i++)
        {
            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = iblSpecular.renderPass[i % iblSpecular.mipmapLevel];
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &iblSpecular.imageAttachment[i].view;
            fbufCreateInfo.width = iblSpecular.mipmapLevelSize[i % iblSpecular.mipmapLevel];
            fbufCreateInfo.height = iblSpecular.mipmapLevelSize[i % iblSpecular.mipmapLevel];

            fbufCreateInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &iblSpecular.frameBuffer[i]))
            {
                throw std::runtime_error("failed create frame buffer");
            }
        }

        for (int i = 0; i < iblSpecular.mappedBuffers.size(); i++)
        {
            createUniformBuffer(1, &iblSpecular.mappedBuffers[i], sizeof(MatricesUBO));
        }

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].pImmutableSamplers = nullptr;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[1].pImmutableSamplers = nullptr;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblSpecular.prePassLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        for (int i = 0; i < iblSpecular.descriptorSets.size(); i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &iblSpecular.prePassLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &iblSpecular.descriptorSets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            //キューブマッピングで使う行列のバッファを流用する
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = cubemapData.mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            //6つのレイヤーを持つ画像を参照するようにする
            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = cubemapData.multiTexture->view;
            info.sampler = cubemapData.multiTexture->sampler;

            std::vector<VkWriteDescriptorSet> descriptorWrites(2);
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = iblSpecular.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = iblSpecular.descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &info;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
     
        //ここで指定されるシェーダはiblのテクスチャ作成用
        createCalcIBLMapPipeline(iblSpecular.vertShaderPath, iblSpecular.fragShaderPath, iblSpecular.prePassLayout,iblSpecular.pipelineLayout
            , static_cast<uint32_t>(iblSpecular.pipeline.size()), iblSpecular.pipeline.data(), static_cast<uint32_t>(iblSpecular.renderPass.size()), iblSpecular.renderPass.data());
    }

    //6枚のテクスチャを作成して、キューブマップを作成
    void VulkanBase::createSamplerCube2D(OffScreenPass& passData,std::vector<MappedBuffer>& mappedBuffers)
    {
        auto c = vkResetFences(device, 1, &inFlightFences[0]);
        if (c != VK_SUCCESS)
        {
            throw std::runtime_error("aa");
        }

        auto e = vkResetCommandBuffer(commandBuffers[0], 0);
        if (e != VK_SUCCESS)
        {
            throw std::runtime_error("AA");
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        Storage* storage = Storage::GetInstance();
        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            //カメラを立方体の内側から、6つの面を映し出すように、ビュー行列を更新していく

            MatricesUBO ubo{};
            ubo.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
            switch (i)
            {
            case CUBEMAP_FACE_FRONT:
                ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
            case CUBEMAP_FACE_BACK:
                ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
            case CUBEMAP_FACE_RIGHT:
                ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
            case CUBEMAP_FACE_LEFT:
                ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
            case CUBEMAP_FACE_TOP:
                ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case CUBEMAP_FACE_BOTTOM:
                ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
                break;
            }

            memcpy(mappedBuffers[i].uniformBufferMapped, &ubo, sizeof(MatricesUBO));

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = passData.renderPass;
            renderPassInfo.framebuffer = passData.frameBuffer[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent.width = passData.width;
            renderPassInfo.renderArea.extent.height = passData.height;

            VkClearValue clearValue{};
            clearValue.depthStencil = { 1.0f,0 };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearValue;

            vkCmdBeginRenderPass(commandBuffers[0], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            drawSamplerCube(storage->getgltfModel(cubemapPath)->getRootNode(), storage->getCubeMap()
                , passData.width, passData.width, commandBuffers[0], i, passData.descriptorSets
                , passData.pipelineLayout, passData.pipeline);

            vkCmdEndRenderPass(commandBuffers[0]);
        }

        if (vkEndCommandBuffer(commandBuffers[0]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        auto b = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[0]);
        if (b != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        auto a = vkWaitForFences(device, 1, &inFlightFences[0], VK_TRUE, UINT64_MAX);
        if (a != VK_SUCCESS)
        {
            throw std::runtime_error("wait failed");
        };
    }

    //specular用のSamplerCubeを作る関数
    void VulkanBase::createSamplerCube2D(IBLSpecularReflection& iblSpecular, std::vector<MappedBuffer>& mappedBuffers)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        SpecularPushConstant constant;

        Storage* storage = Storage::GetInstance();
        //6つの面をレンダリングする
        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            {//カメラを立方体の内側から、6つの面を映し出すように、ビュー行列を更新していく

                MatricesUBO ubo{};
                ubo.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
                switch (i)
                {
                case CUBEMAP_FACE_FRONT:
                    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                    break;
                case CUBEMAP_FACE_BACK:
                    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                    break;
                case CUBEMAP_FACE_RIGHT:
                    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                    break;
                case CUBEMAP_FACE_LEFT:
                    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                    break;
                case CUBEMAP_FACE_TOP:
                    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                    break;
                case CUBEMAP_FACE_BOTTOM:
                    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
                    break;
                }

                memcpy(mappedBuffers[i].uniformBufferMapped, &ubo, sizeof(MatricesUBO));
            }

            //一つの面に対して、複数種類の解像度でレンダリングする
            for (uint32_t j = 0; j < iblSpecular.mipmapLevel; j++)
            {
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = iblSpecular.renderPass[j];
                renderPassInfo.framebuffer = iblSpecular.frameBuffer[j + (i * iblSpecular.mipmapLevel)];
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent.width = iblSpecular.mipmapLevelSize[j];
                renderPassInfo.renderArea.extent.height = iblSpecular.mipmapLevelSize[j];

                VkClearValue clearValue{};
                clearValue.depthStencil = { 1.0f,0 };
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clearValue;

                vkCmdBeginRenderPass(commandBuffers[0], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                //ミップマップレベルに応じて、roughnessとしてシェーダに値を渡して、BRDFの値を調整する
                constant.roughness = static_cast<float>(j) / static_cast<float>(iblSpecular.mipmapLevel);
                vkCmdPushConstants(commandBuffers[0], iblSpecular.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SpecularPushConstant), &constant);

                drawSamplerCube(storage->getgltfModel(cubemapPath)->getRootNode(), storage->getCubeMap()
                    ,iblSpecular.mipmapLevelSize[j], iblSpecular.mipmapLevelSize[j], commandBuffers[0], i,iblSpecular.descriptorSets
                    ,iblSpecular.pipelineLayout,iblSpecular.pipeline[j]);

                vkCmdEndRenderPass(commandBuffers[0]);
            }
        }

        if (vkEndCommandBuffer(commandBuffers[0]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        vkResetFences(device, 1, &inFlightFences[0]);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[0]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        vkWaitForFences(device, 1, &inFlightFences[0], VK_TRUE, UINT64_MAX);
    }

    //specular用のSamplerCubeを作る関数
    void VulkanBase::createLUT(IBLSpecularBRDF& iblSpecular, MappedBuffer& mappedBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        Storage* storage = Storage::GetInstance();

        //一つの面のみレンダリングする
        MatricesUBO ubo{};
        ubo.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        memcpy(mappedBuffer.uniformBufferMapped, &ubo, sizeof(MatricesUBO));

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = iblSpecular.passData.renderPass;
        renderPassInfo.framebuffer = iblSpecular.passData.frameBuffer[0];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent.width = iblSpecular.passData.width;
        renderPassInfo.renderArea.extent.height = iblSpecular.passData.height;

        VkClearValue clearValue{};
        clearValue.depthStencil = { 1.0f,0 };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffers[0], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        drawSamplerCube(storage->getgltfModel(cubemapPath)->getRootNode(), storage->getCubeMap()
            , iblSpecular.passData.width, iblSpecular.passData.height, commandBuffers[0], 0/*一つの面しかレンダリングしないので0で固定*/, iblSpecular.passData.descriptorSets
            , iblSpecular.passData.pipelineLayout, iblSpecular.passData.pipeline);

        vkCmdEndRenderPass(commandBuffers[0]);

        if (vkEndCommandBuffer(commandBuffers[0]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        vkResetFences(device, 1, &inFlightFences[0]);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[0]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        vkWaitForFences(device, 1, &inFlightFences[0], VK_TRUE, UINT64_MAX);
    }

    //キューブマップを作成するための画像をレンダリング
    void VulkanBase::drawSamplerCube(GltfNode* node, std::shared_ptr<Model> model
        ,uint32_t width,uint32_t height, VkCommandBuffer& commandBuffer, int index,std::vector<VkDescriptorSet>& descriptorSets
        ,VkPipelineLayout& pipelineLayout,VkPipeline& pipeline)
    {
        for(auto mesh : node->meshArray)
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)width;
            viewport.height = (float)height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = { width, height };
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffer, model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, static_cast<uint32_t>(1), &descriptorSets[index], 0, nullptr);

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            drawSamplerCube(node->children[i], model, width, height, commandBuffer, index, descriptorSets,pipelineLayout,pipeline);
        }
    }

    void VulkanBase::createMultiLayerTexture(TextureData* dstTextureData, uint32_t layerCount
        ,uint32_t width,uint32_t height,uint32_t mipLevel,VkFormat format)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevel;
        imageInfo.arrayLayers = layerCount;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if (vkCreateImage(device, &imageInfo, nullptr, &dstTextureData->image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(device, dstTextureData->image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &dstTextureData->memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, dstTextureData->image, dstTextureData->memory, 0);
    }

    //キューブマップの作成
    void VulkanBase::setCubeMapModel(std::shared_ptr<Model> cubemap)
    {
        cubemapData.format = VK_FORMAT_R32G32B32A32_SFLOAT;

        prepareCubemapTextures();//キューブマッピングの用意

        /*頂点、インデックスバッファーを持たせる*/
        createMeshesData(cubemap);

        /*UnifomBufferを持たせる*/
        createUniformBuffers(cubemap);

        //6枚のテクスチャを作成する
        createSamplerCube2D(cubemapData.passData,cubemapData.mappedBuffers);

        /*スカイボックス用のテクスチャを用意する*/
        {
            {
                //スカイボックス用にレイヤーの数を6つに
                std::shared_ptr<ImageData> image = Storage::GetInstance()->getCubemapImage();
                int cubemapImageSize = std::min(image->getWidth(), image->getHeight());

                //フレームバッファからキューブマップのレンダリング結果を取得し
                //フレームバッファの画像のレイアウトを送信用にする
                {
                    for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
                    {
                        transitionImageLayout(cubemapData.passData.imageAttachment[i].image,
                            cubemapData.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);
                    }
                }

                cubemapData.multiTexture->mipLevel = calcMipMapLevel(cubemapImageSize, cubemapImageSize);

                {//6つのレイヤーを持つテクスチャデータを作成
                    createMultiLayerTexture(cubemapData.multiTexture, CUBEMAP_FACE_COUNT
                        , cubemapImageSize, cubemapImageSize, cubemapData.multiTexture->mipLevel,cubemapData.format);

                    //画像のすべてのレイヤーを移動させる
                    transitionImageLayout(cubemapData.multiTexture->image, cubemapData.format
                        , VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cubemapData.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);

                    std::vector<VkImage> srcImages(CUBEMAP_FACE_COUNT);
                    for (uint32_t i = 0;i < cubemapData.passData.imageAttachment.size();i++)
                    {
                        srcImages[i] = cubemapData.passData.imageAttachment[i].image;
                    }
                    copyImageToMultiLayerImage(srcImages.data(), static_cast<uint32_t>(srcImages.size())
                        , cubemapData.passData.width, cubemapData.passData.height, cubemapData.multiTexture->image);

                    generateMipmaps(cubemapData.multiTexture->image, cubemapData.format
                        , cubemapImageSize, cubemapImageSize, cubemapData.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);
                }
            }

            //ビューをテクスチャキューブに設定
            cubemapData.multiTexture->view = createImageView(cubemapData.multiTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, cubemapData.format, VK_IMAGE_ASPECT_COLOR_BIT
                , cubemapData.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);

            createTextureSampler(cubemapData.multiTexture);
        }

        {
            //キューブマッピングを反映するためのdescriptorSetのレイアウトを作成

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 0;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &samplerLayoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &cubemapData.layout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        createCubemapPipeline("shaders/cubemap.vert.spv", "shaders/cubemap.frag.spv", cubemapData.layout, cubemapData.pipelineLayout, cubemapData.pipeline, renderPass);

        /*ここからパイプラインは、同じグループのモデルでは使いまわせる*/
        /*ディスクリプタセットは、テクスチャデータが異なる場合は使いまわせない*/
        createDescriptorInfos(cubemapData.pipelineLayout, cubemapData.pipeline, cubemap);

        //cubemapのテクスチャ部分のdescriptorSetのメモリーを確保
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &cubemapData.layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &cubemapData.descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;
        }

        //SamplerCube用のdescriptorSetの作成
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = cubemapData.multiTexture->view;
            imageInfo.sampler = cubemapData.multiTexture->sampler;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = cubemapData.descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }

        /*ディスクリプタ用のメモリを空ける*/
        allocateDescriptorSets(modelDescriptor.layout, cubemap);//マテリアルが複数ある場合エラー

        /*MatricesUBOのdescriptorSetを作る*/
        createDescriptorSet_CubeMap(cubemap->getRootNode(), cubemap);

        //IBLを作成する
        createIBL();

        cubemap->getGltfModel()->setup = true;
    }

    //IBLを作成する
    void VulkanBase::createIBL()
    {
        //diffuse
        iblDiffuse.format = cubemapData.format;
        createIBLDiffuse();
        //specular
        iblSpecularReflection.format = cubemapData.format;
        createIBLSpecular();
    }

    //IBLのdiffuseテクスチャを作成する
    void VulkanBase::createIBLDiffuse()
    {
        //事前計算用のシェーダの設定
        iblDiffuse.vertShaderPath = "shaders/calcIBL.vert.spv";
        iblDiffuse.fragShaderPath = "shaders/calcDiffuse.frag.spv";

        //オフスクリーンレンダリング用のフレーム数を作成する
        iblDiffuse.setFrameCount(CUBEMAP_FACE_COUNT);
        //テクスチャのサイズ設定
        iblDiffuse.setRenderSize(IBL_MAP_SIZE);

        //ミップマップレベルの計算
        iblDiffuse.setMipmapLevel(1);

        //diffuse用テクスチャの6回のオフスクリーンレンダリングの準備を行う
        prepareIBL(iblDiffuse.vertShaderPath, iblDiffuse.fragShaderPath
            , iblDiffuse.passData, iblDiffuse.format,iblDiffuse.mipmapLevel,iblDiffuse.mappedBuffers);

        //SamperCubeを作成する
        createSamplerCube2D(iblDiffuse.passData, iblDiffuse.mappedBuffers);

        //6つの視点の画像をまとめて、SamplerCubeを作成する
        createCubeMapTextureFromImages(iblDiffuse.size, iblDiffuse.mipmapLevel, iblDiffuse.multiLayerTexture, iblDiffuse.passData.imageAttachment, iblDiffuse.format);

        //通常レンダリング時に使うDescriptorSet関連のデータを作成
        createIBLDescriptor(iblDiffuse.multiLayerTexture, iblDiffuse.mainPassLayout, iblDiffuse.descriptorSet);
    }

    //IBLのspecularテクスチャを作成する
    void VulkanBase::createIBLSpecular()
    {
        //specularの鏡面反射項のマップを作成
        createIBLSpecularReflection();
        //specularのBRDFのマップを作成
        createIBLSpecularBRDF();
    }

    void VulkanBase::createIBLSpecularReflection()
    {
        //事前計算用のシェーダを設定
        iblSpecularReflection.vertShaderPath = "shaders/calcIBL.vert.spv";
        iblSpecularReflection.fragShaderPath = "shaders/calcSpecularReflection.frag.spv";

        //テクスチャのサイズの設定
        iblSpecularReflection.setRenderSize(IBL_MAP_SIZE);

        //オフスクリーンレンダリングのフレーム数を設定する
        iblSpecularReflection.setFrameCount(CUBEMAP_FACE_COUNT);

        //specular用テクスチャの6回のオフスクリーンレンダリングの準備を行う
        prepareIBL(iblSpecularReflection);

        //6つの視点からの立方体の内側をレンダリングして、6つの視点それぞれの画像をミップマップの数だけ作成
        createSamplerCube2D(iblSpecularReflection, iblSpecularReflection.mappedBuffers);

        //6つの視点の画像をまとめて、SamplerCubeを作成する
        createCubeMapTextureFromImages(iblSpecularReflection.size, iblSpecularReflection.multiLayerTexture
            , iblSpecularReflection.imageAttachment, iblSpecularReflection.mipmapLevelSize,iblSpecularReflection.format);

        //通常レンダリング時に使うDescriptorSet関連のデータを作成
        createIBLDescriptor(iblSpecularReflection.multiLayerTexture, iblSpecularReflection.mainPassLayout, iblSpecularReflection.descriptorSet);
    }

    //specularのBRDF項についてのマップを作成
    void VulkanBase::createIBLSpecularBRDF()
    {
        //事前計算用のシェーダを設定
        iblSpecularBRDF.vertShaderPath = "shaders\\calcBRDF.vert.spv";
        iblSpecularBRDF.fragShaderPath = "shaders\\calcSpecularBRDF.frag.spv";

        //テクスチャのサイズの設定
        iblSpecularBRDF.setRenderSize(IBL_MAP_SIZE);

        //IBLのspeuclarのBRDFでは、立方体の一面のみレンダリングすればいい
        iblSpecularBRDF.setFrameCount(1);

        //specular用テクスチャの6回のオフスクリーンレンダリングの準備を行う
        prepareIBL(iblSpecularBRDF.vertShaderPath, iblSpecularBRDF.fragShaderPath
            , iblSpecularBRDF.passData, VK_FORMAT_R16G16B16A16_SFLOAT,1,iblSpecularBRDF.mappedBuffers);

        //一つの視点のみから立方体の面をレンダリングして、2DのLUTを作成する
        createLUT(iblSpecularBRDF, iblSpecularBRDF.mappedBuffers[0]);

        //レンダリングした画像をシェーダで読み取るためのテクスチャに変換する
        transitionImageLayout(iblSpecularBRDF.passData.imageAttachment[0].image, VK_FORMAT_R16G16B16A16_SFLOAT
            , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

        //立方体の一つの面のみのレンダリング結果をLUTとして利用
        //通常レンダリング時に使うDescriptorSet関連のデータを作成
        createIBLDescriptor(iblSpecularBRDF.passData, iblSpecularBRDF.mainPassLayout, iblSpecularBRDF.descriptorSet);
    }

    //6つの画像を一つの画像にまとめて、SamplerCubeを作る
    void VulkanBase::createCubeMapTextureFromImages(uint32_t texSize,uint32_t srcTextureMipmapLevel, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment,VkFormat format)
    {
        //6つのレイヤーをもつ画像を作る
        createMultiLayerTexture(multiLayerTexture, CUBEMAP_FACE_COUNT, texSize, texSize, multiLayerTexture->mipLevel,format);

        //レンダリングした6枚の画像を転送用のレイアウトに変更する
        for (int i = 0; i < imageAttachment.size(); i++)
        {
            transitionImageLayout(imageAttachment[i].image,
                format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcTextureMipmapLevel, 1);
        }

        //SamplerCubeの画像のすべてのレイヤーを移動させる
        transitionImageLayout(multiLayerTexture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        //今まで作成した解像度の違う画像を、同一の視点の画像のミップマップとして格納する
        //同じ視点のレンダリング画像を集めて、それをミップマップとしてまとめている
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
            {
                //あらかじめレンダリングしておいた各面のspecularのテクスチャの素材を
                //ここで、6つのレイヤーを持ち、複数のミップマップをもつ画像データにひとまとめにする

                VkImageCopy region{};
                region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.srcSubresource.mipLevel = 0;
                region.srcSubresource.baseArrayLayer = 0;
                region.srcSubresource.layerCount = 1;
                region.srcOffset = { 0, 0, 0 };
                region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.dstSubresource.mipLevel = 0;
                region.dstSubresource.baseArrayLayer = i;
                region.dstSubresource.layerCount = 1;
                region.dstOffset = { 0, 0, 0 };
                region.extent = { texSize, texSize, 1 };

                vkCmdCopyImage(commandBuffer, imageAttachment[i].image
                    , VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, multiLayerTexture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            }

            endSingleTimeCommands(commandBuffer);
        }

        //ミップマップの作成
        generateMipmaps(multiLayerTexture->image, format,
            texSize, texSize, multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        //ビューをテクスチャキューブに設定
        multiLayerTexture->view = createImageView(multiLayerTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, format, VK_IMAGE_ASPECT_COLOR_BIT
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        createTextureSampler(multiLayerTexture);
    }

    //6つの画像を一つの画像にまとめて、SamplerCubeを作る
    void VulkanBase::createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, std::vector<uint32_t>& mipmapLevelSize,VkFormat format)
    {
        //6つのレイヤーをもつ画像を作る
        createMultiLayerTexture(multiLayerTexture, CUBEMAP_FACE_COUNT, texSize, texSize, multiLayerTexture->mipLevel,format);

        //レンダリングした6枚の画像を転送用のレイアウトに変更する
        for (int i = 0; i < imageAttachment.size(); i++)
        {
            transitionImageLayout(imageAttachment[i].image,
                format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);
        }

        //SamplerCubeの画像のすべてのレイヤーを移動させる
        transitionImageLayout(multiLayerTexture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);


        //今まで作成した解像度の違う画像を、同一の視点の画像のミップマップとして格納する
        //同じ視点のレンダリング画像を集めて、それをミップマップとしてまとめている
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
            {
                //あらかじめレンダリングしておいた各面のspecularのテクスチャの素材を
                //ここで、6つのレイヤーを持ち、複数のミップマップをもつ画像データにひとまとめにする

                for (uint32_t j = 0; j < multiLayerTexture->mipLevel; j++)
                {
                    VkImageCopy region{};
                    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.srcSubresource.mipLevel = 0;
                    region.srcSubresource.baseArrayLayer = 0;
                    region.srcSubresource.layerCount = 1;
                    region.srcOffset = { 0, 0, 0 };
                    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.dstSubresource.mipLevel = j;
                    region.dstSubresource.baseArrayLayer = i;
                    region.dstSubresource.layerCount = 1;
                    region.dstOffset = { 0, 0, 0 };
                    region.extent = { mipmapLevelSize[j], mipmapLevelSize[j], 1 };

                    vkCmdCopyImage(commandBuffer, imageAttachment[j + (i * multiLayerTexture->mipLevel)].image
                        , VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, multiLayerTexture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
                }
            }

            endSingleTimeCommands(commandBuffer);
        }

        //ミップマップの作成
        generateMipmaps(multiLayerTexture->image, format,
            texSize, texSize, multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        //ビューをテクスチャキューブに設定
        multiLayerTexture->view = createImageView(multiLayerTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, format, VK_IMAGE_ASPECT_COLOR_BIT
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        createTextureSampler(multiLayerTexture);
    }

    //IBL用のDescriptorSetの用意
    void VulkanBase::createIBLDescriptor(TextureData* samplerCube, VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet)
    {
        //DescriptorSetLayoutの作成
        if(!layout)
        {
            VkDescriptorSetLayoutBinding layoutBinding;
            layoutBinding.binding = 0;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &layoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        {//DescriptorSetの実態を作成
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = samplerCube->view;
            imageInfo.sampler = samplerCube->sampler;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    //IBL用のDescriptorSetの用意(LUT用)
    void VulkanBase::createIBLDescriptor(OffScreenPass& passData, VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet)
    {
        //DescriptorSetLayoutの作成
        if (!layout)
        {
            VkDescriptorSetLayoutBinding layoutBinding;
            layoutBinding.binding = 0;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &layoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        {//DescriptorSetの実態を作成
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            if (passData.imageAttachment.size() != 1)
            {
                throw std::runtime_error("failed to imageAttachment count!");
            }

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = passData.imageAttachment[0].view;
            imageInfo.sampler = passData.sampler;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    //UIの頂点バッファなどを用意する
    void VulkanBase::setUI(std::shared_ptr<UI> ui)
    {
        //頂点バッファの作成
        createVertexBuffer(ui);
        //インデックスバッファの作成
        createIndexBuffer(ui);

        //VkDescriptorSetのバッファを確保
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &uiRender.layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &ui->getDescriptorSet()) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;
        }

        //バッファーを結び付ける
        changeUITexture(ui->getUITexture(), ui->getMappedBuffer(), ui->getDescriptorSet());
    }

    //UIのuniform bufferの作成
    void VulkanBase::uiCreateUniformBuffer(MappedBuffer& mappedBuffer)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO2D);

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer.uniformBuffer, mappedBuffer.uniformBufferMemory);

        vkMapMemory(device, mappedBuffer.uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer.uniformBufferMapped);
    }

    //UIのテクスチャの変更を反映
    void VulkanBase::changeUITexture(TextureData* textureData, MappedBuffer& mappedBuffer, VkDescriptorSet& descriptorSet)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mappedBuffer.uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatricesUBO2D);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureData->view;
        imageInfo.sampler = textureData->sampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize(2);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    //レンダリングの開始
    void VulkanBase::render()
    {
        descriptorSetCount = 0;

        /*描画*/
        drawFrame();
    }