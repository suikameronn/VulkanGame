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

    //Vulkan�̏�����
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
        //createUniformBuffers();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorPool();
        createEmptyImage();
    }

    //�X���b�v�`�F�[���̔j��
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

    //�Q�[���I�����Ƀf�[�^��gpu��̃f�[�^�����ׂĔj������
    void VulkanBase::cleanup()
    {
        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        emptyImage.destroy(device);
        shadowMapData.destroy(device);
        modelDescriptor.destroy(device);
        cubemapData.destroy(device);

        //IBL�̃f�[�^�̔j��
        iblDiffuse.destroy(device);
        iblSpecularReflection.destroy(device);
        iblSpecularBRDF.destroy(device);

        Storage* storage = Storage::GetInstance();

        for (auto gltfModel:storage->getgltfModel())//gltf���f���̃o�b�t�@�[�Ȃǂ̍폜
        {
            gltfModel.second->cleanUpVulkan(device);
        }

        storage->getCubeMap()->cleanupVulkan();//�L���[�u�}�b�v�p�̃o�b�t�@�[�Ȃǂ̍폜

        for (auto model:storage->getModels())//Model�N���X�̃o�b�t�@�[�Ȃǂ̍폜
        {
            model->cleanupVulkan();
        }

        storage->getPointLightsBuffer().destroy(device);//�|�C���g���C�g�̃o�b�t�@�̌㏈��
        storage->getDirectionalLightsBuffer().destroy(device);//�f�B���N�V���i�����C�g�̃o�b�t�@�̌㏈��

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        //���̎��_��gpu���ɍ�����ϐ���j�����Ēu���Ȃ���΂Ȃ�Ȃ�
        //device�̕ϐ����g���č��ϐ����j��
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    //�E�B���h�E�T�C�Y���ς�����ۂ̃X���b�v�`�F�[���̉摜�T�C�Y�̕ύX
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

    //�C���X�^���X�̍쐬
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
        appInfo.apiVersion = VK_API_VERSION_1_0;

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

    //�����_�[��̏o�͐�̃E�B���h�E�̗p��
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

    //Vulkan�ň����f�o�C�X��I������
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

    //Vulkan��ň����f�o�C�X�f�[�^�̍쐬
    void VulkanBase::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
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
    }

    //�X���b�v�`�F�[���̍쐬
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

    //�����_�[�p�X�̍쐬
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

    //MVP�s��ƃA�j���[�V�����s����܂�descriptorSetLayout���쐬
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

    //ShaderMaterial���܂�descriptorSetLayout�̍쐬
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

    //�ʏ�̃����_�����O�Ŏg�p����p�C�v���C���̍쐬
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
        layouts[0] = layout;//MVP�s��ƃA�j���[�V�����s��
        layouts[1] = modelDescriptor.materialLayout;//�}�e���A��
        layouts[2] = modelDescriptor.lightLayout;//�|�C���g���C�g
        layouts[3] = modelDescriptor.lightLayout;//���s����
        layouts[4] = shadowMapData.layout;//�V���h�E�}�b�v
        layouts[5] = iblDiffuse.mainPassLayout;//IBL��diffuse�p
        layouts[6] = iblSpecularReflection.mainPassLayout;//IBL��specular�̋��ʔ��˗p
        layouts[7] = iblSpecularBRDF.mainPassLayout;//IBL��specular��BRDF�p

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

    //�V���h�E�}�b�v�쐬�p�̃p�C�v���C�����쐬
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
        colorBlending.attachmentCount = 0;//�J���[�A�^�b�`�����g�͂���Ȃ�
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
            VK_DYNAMIC_STATE_DEPTH_BIAS//�f�v�X�o�b�t�@�̒l�𒲐��ł���悤�ɂ���
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

    //HDRI�摜����L���[�u�}�b�v�p�̃p�C�v���C���̍쐬
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

    //�L���[�u�}�b�v�̃����_�����O�p�̃p�C�v���C���̍쐬
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
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;

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

    //IBL�̃}�b�v�v�Z�p�̃p�C�v���C���̍쐬
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
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord0);

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

        //PipelneLayout���d�����č쐬����̂��������
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

    //�ʏ�̃����_�����O�Ŏd�l����t���[���o�b�t�@�̍쐬
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

    //�R�}���h�v�[���̍쐬 �R�}���h�o�b�t�@�[�͂��ꂩ����
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
    }

    //�ʏ�̃����_�����O�Ŏg�p����J���[�A�^�b�`�����g�̍쐬
    void VulkanBase::createColorResources()
    {
        VkFormat colorFormat = swapChainImageFormat;

        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);

        colorImageView = createImageView(colorImage, VK_IMAGE_VIEW_TYPE_2D, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1,1);
    }

    //�ʏ�̃����_�����O�Ŏg�p����f�v�X�A�^�b�`�����g�̍쐬
    void VulkanBase::createDepthResources()
    {
        VkFormat depthFormat = findDepthFormat();

        createImage(swapChainExtent.width, swapChainExtent.height,1,msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, VK_IMAGE_VIEW_TYPE_2D, depthFormat,VK_IMAGE_ASPECT_DEPTH_BIT,1,1);

        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,1,1);

    }

    //gpu���T�|�[�g���Ă���摜�t�H�[�}�b�g�̒T��
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

    //gpu���T�|�[�g���Ă���f�v�X�C���[�W�̃t�H�[�}�b�g�̒T��
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

    //�摜�̃T�C�Y����쐬�\�ȃ~�b�v�}�b�v���x���̌v�Z
    uint32_t VulkanBase::calcMipMapLevel(uint32_t width, uint32_t height)
    {
        return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
    }

    //�P��̃��C���[��VkImage�𕡐��̃��C���[��VkImage�ɃR�s�[����
    void VulkanBase::copyImageToMultiLayerImage(VkImage* srcImages, uint32_t imageCount,uint32_t width,uint32_t height, VkImage& dstImage)
    {
        //�R�}���h�̋L�^�J�n
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        //VkImage�𖇐������VkImage�ɃR�s�[
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

        //�R�}���h�o�b�t�@�̋L�^���I������
        endSingleTimeCommands(commandBuffer);

    }

    //�摜����e�N�X�`���摜�̍쐬 
    void VulkanBase::createTextureImage(TextureData& textureData,std::shared_ptr<ImageData> image)
    {
        VkDeviceSize bufferSize = image->getWidth() * image->getHeight() * image->getTexChannels();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, image->getPixelsData(), bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        textureData.mipLevel = calcMipMapLevel(image->getWidth(),image->getHeight());
        createImage(image->getWidth(), image->getHeight(), textureData.mipLevel, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureData.image, textureData.memory);

        transitionImageLayout(textureData.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureData.mipLevel,1);
        copyBufferToImage(stagingBuffer, textureData.image, static_cast<uint32_t>(image->getWidth()), static_cast<uint32_t>(image->getHeight()),1);

        generateMipmaps(textureData.image, VK_FORMAT_R8G8B8A8_UNORM, image->getWidth(), image->getHeight(), textureData.mipLevel,1);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //�_�~�[�e�N�X�`���̍쐬
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

    //gltf���f���̃}�e���A���̃e�N�X�`���̍쐬
    void VulkanBase::createTextureImage(std::shared_ptr<GltfModel> gltfModel)
    {
        if (gltfModel->textureDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->textureDatas.size(); i++)
        {
            std::shared_ptr<ImageData> imageData = gltfModel->imageDatas[i];
            TextureData* textureData = gltfModel->textureDatas[i];

            VkDeviceSize imageSize = imageData->getWidth() * imageData->getHeight() * imageData->getTexChannels();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, imageData->getPixelsData(), static_cast<size_t>(imageSize));
            vkUnmapMemory(device, stagingBufferMemory);

            textureData->mipLevel = calcMipMapLevel(imageData->getWidth(), imageData->getHeight());
            createImage(imageData->getWidth(), imageData->getHeight(), textureData->mipLevel, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
                , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureData->image, textureData->memory);

            transitionImageLayout(textureData->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureData->mipLevel,1);
            copyBufferToImage(stagingBuffer, textureData->image, static_cast<uint32_t>(imageData->getWidth()), static_cast<uint32_t>(imageData->getHeight()),1);

            generateMipmaps(textureData->image, VK_FORMAT_R8G8B8A8_UNORM, imageData->getWidth(), imageData->getHeight(), textureData->mipLevel,1);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }
    }

    //�~�b�v�}�b�v�摜�̍쐬
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

    //�e�N�X�`���̃r���[�̍쐬
    void VulkanBase::createTextureImageView(TextureData& textureData)
    {
        textureData.view = createImageView(textureData.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureData.mipLevel,1);
    }

    //�_�~�[�e�N�X�`���p
    void VulkanBase::createTextureImageView()
    {
        TextureData* textureData = emptyImage.emptyTex;
        textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
    }

    //gltf���f���̃e�N�X�`���p�̃r���[�̍쐬
    void VulkanBase::createTextureImageView(std::shared_ptr<GltfModel> gltfModel) 
    {
        if (gltfModel->textureDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->textureDatas.size(); i++)
        {
            TextureData* textureData = gltfModel->textureDatas[i];
            textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
        }
    }

    //�e�N�X�`���̃T���v���[�̍쐬
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

    //�_�~�[�e�N�X�`���̃T���v���[�̍쐬
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

    //gltf���f���̃e�N�X�`���̃T���v���[�̍쐬
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

    //�e�N�X�`���̃��C�A�E�g��ύX����
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

        //���C�A�E�g���o�͐�ɕύX����
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        //�o�͂��ꂽ�e�N�X�`�����V�F�[�_��ŗ��p�ł���悤�ɂ���
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        //�f�v�X�C���[�W�p�ɂ���
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        //�J���[�A�^�b�`�����g�p�̃��C�A�E�g�𑗐M�p�̃��C�A�E�g�ɕύX����
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        //�J���[�A�^�b�`�����g�p�̃��C�A�E�g���V�F�[�_�Ŏg�����߂̃��C�A�E�g�ɂ���
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
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

    //�s�N�Z���̃o�b�t�@�[����e�N�X�`���摜���쐬����
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

    //Model�N���X�p�̒��_�o�b�t�@�[���쐬����
    void VulkanBase::createVertexBuffer(GltfNode* node,std::shared_ptr<Model> model)
    {
        if (node->mesh)
        {
            Mesh* mesh = node->mesh;
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

            //vertexBuffer�z��ɃR�s�[���Ă���(vector�^)
            copyBuffer(stagingBuffer, model->getPointBufferData()[mesh->meshIndex].vertBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        for (size_t i = 0; i < node->children.size(); i++)
        {
            createVertexBuffer(node->children[i], model);
        }
    }

    //�R���C�_�[�p�̒��_�o�b�t�@�[���쐬
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

        //vertexBuffer�z��ɃR�s�[���Ă���(vector�^)
        copyBuffer(stagingBuffer, colider->getPointBufferData()->vertBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //Model�N���X�p�̃C���f�b�N�X�o�b�t�@�[���쐬����
    void VulkanBase::createIndexBuffer(GltfNode* node, std::shared_ptr<Model> model)
    {
        if(node->mesh)
        {
            Mesh* mesh = node->mesh;
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

    //�R���C�_�[�p�̃C���f�b�N�X�o�b�t�@�[���쐬
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

    //���j�t�H�[���o�b�t�@�̍쐬
    void VulkanBase::createUniformBuffer(int count,MappedBuffer* mappedBuffer,unsigned long long size)
    {
        VkDeviceSize bufferSize = size * count;

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //Model�N���X��MVP�s��p�̃��j�t�H�[���o�b�t�@�̍쐬
    void VulkanBase::createUniformBuffer(std::shared_ptr<Model> model)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO);

        MappedBuffer* mappedBuffer = &model->getModelViewMappedBuffer();

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //gltf���f���̃m�[�h�̐������A�j���[�V�����s��̃o�b�t�@�[���쐬����
    void VulkanBase::createUniformBuffer(GltfNode* node, std::shared_ptr<Model> model)
    {
        if (node->mesh)
        {
            Mesh* mesh = node->mesh;

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

    //�R���C�_�[�p��MVP�s��̃o�b�t�@�[���쐬
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

    //�}�e���A���p�̃o�b�t�@�[�̍쐬
    void VulkanBase::createUniformBuffer(std::shared_ptr<Material> material)
    {
        VkDeviceSize bufferSize = sizeof(ShaderMaterial);

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            , material->sMaterialMappedBuffer.uniformBuffer, material->sMaterialMappedBuffer.uniformBufferMemory);

        vkMapMemory(device, material->sMaterialMappedBuffer.uniformBufferMemory, 0, bufferSize, 0, &material->sMaterialMappedBuffer.uniformBufferMapped);
    }

    //MVP�s��ƃA�j���[�V�����s��̃o�b�t�@�[�̍쐬 �t������R���C�_�[�̏ꍇ�͂�����쐬
    void VulkanBase::createUniformBuffers(std::shared_ptr<Model> model)
    {
        createUniformBuffer(model);

        createUniformBuffer(model->getRootNode(), model);

        if (model->hasColider())
        {
            createUniformBuffer(model->getColider());
        }
    }

    //�|�C���g���C�g�̍s��̍X�V
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

    //���s�����̍s��̍X�V
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

    //Model�N���X�̃A�j���[�V�����s��̍X�V������
    void VulkanBase::updateUniformBuffer(GltfNode* node, std::shared_ptr<Model> model)
    {
        if (node->mesh)
        {
            Mesh* mesh = node->mesh;

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

    //�V���h�E�}�b�v�쐬�p�̍s��̍X�V
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

    //Model�N���X��MVP�s��̍X�V
    void VulkanBase::updateUniformBuffers(std::shared_ptr<Model> model)
    {
        Storage* storage = Storage::GetInstance();
        std::shared_ptr<Camera> camera = storage->accessCamera();

        MatricesUBO ubo;

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

            MatricesUBO ubo;

            ubo.model = model->getTransformMatrix();
            ubo.view = camera->viewMat;
            ubo.proj = camera->perspectiveMat;
            ubo.worldCameraPos = glm::vec4(camera->getPosition(), 1.0);

            memcpy(colider->getMappedBufferData()->uniformBufferMapped, &ubo, sizeof(ubo));
        }
    }

    //�L���[�u�}�b�v�`��p�̍s��̍X�V
    void VulkanBase::updateUniformBuffers_Cubemap(std::shared_ptr<Model> cubemap)
    {
        Storage* storage = Storage::GetInstance();
        std::shared_ptr<Camera> camera = storage->accessCamera();

        MatricesUBO ubo;

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

        //vertexBuffer�z��ɃR�s�[���Ă���(vector�^)
        copyBuffer(stagingBuffer, colider->getPointBufferData()->vertBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //descriptorPool�̍쐬 descriptorSet�͂�������쐬
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
        poolInfo.maxSets = static_cast<uint32_t>(100);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    //descriptorSet�̃o�b�t�@�[�̗p��
    //gltf���f���p
    void VulkanBase::allocateDescriptorSet(VkDescriptorSetLayout& layout,GltfNode * node, std::shared_ptr<Model> model)
    {
        if (node->mesh)
        {
            Mesh* mesh = node->mesh;
            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                DescriptorInfo* info = &mesh->descriptorInfo;

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

                if (descriptorSetCount > 100)
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

    //�R���C�_�[�p
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

    //ShaderMaterial�p
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

        if (descriptorSetCount > 100)
        {
            throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
        }
        descriptorSetCount++;

        model->getColider()->setDescriptorSet(descriptorSet);
    }

    //gltf���f���p
    void VulkanBase::allocateDescriptorSets(VkDescriptorSetLayout& layout,std::shared_ptr<Model> model)
    {
        allocateDescriptorSet(layout,model->getRootNode(), model);
        if (model->hasColider())
        {
            allocateDescriptorSet(layout,model);
        }
    }

    //descriptorSet�̍쐬
    //gltf���f���̍쐬
    void VulkanBase::createDescriptorSet(GltfNode* node,std::shared_ptr<Model> model)
    {
        if(node->mesh)
        {
            Mesh* mesh = node->mesh;
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

    //�R���C�_�[�p
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

    //�}�e���A���p
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

    //gltf���f���̍쐬
    void VulkanBase::createDescriptorSets(std::shared_ptr<Model> model)
    {
        createDescriptorSet(model->getRootNode(),model);
        if (model->hasColider())
        {
            createDescriptorSet(model);
        }
    }

    //�L���[�u�}�b�v�p
    void VulkanBase::createDescriptorSet_CubeMap(GltfNode* node, std::shared_ptr<Model> model)
    {
        if (node->mesh)
        {
            Mesh* mesh = node->mesh;
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

    //�o�b�t�@�̍쐬
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

    //�g���̂ẴR�}���h�̋L�^�J�n
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

    //�R�}���h�̋L�^�I��
    void VulkanBase::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    //�o�b�t�@��̃f�[�^�̃R�s�[
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

    //�R�}���h�o�b�t�@�[���쐬����
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

    //�L���[�u�}�b�v�̃����_�����O
    void VulkanBase::drawCubeMap(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer)
    {
        Storage* storage = Storage::GetInstance();

        if (node->mesh)
        {
            Mesh* mesh = node->mesh;

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh->descriptorInfo.pipeline);

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
                    mesh->descriptorInfo.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            drawCubeMap(node->children[i], model, commandBuffer);
        }
    }

    //gltf���f���̕`��
    void VulkanBase::drawMesh(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer)
    {
        Storage* storage = Storage::GetInstance();

        if(node->mesh)
        {
            Mesh* mesh = node->mesh;

            PushConstantObj constant = { node->getMatrix() };

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh->descriptorInfo.pipeline);

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

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffer, model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (int i = 0;i < mesh->primitives.size();i++)
            {
                std::shared_ptr<Material> material = model->getGltfModel()->materials[mesh->primitives[i].materialIndex];

                std::vector<VkDescriptorSet> descriptorSets =
                {
                    model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet,//MVP�s��ƃA�j���[�V�����s��
                    material->descriptorSet,//�}�e���A��
                    storage->getPointLightDescriptorSet(),//�|�C���g���C�g
                    storage->getDirectionalLightDescriptorSet(),//���s����
                    shadowMapData.descriptorSets[0],//�V���h�E�}�b�v
                    iblDiffuse.descriptorSet,//IBL��diffuse
                    iblSpecularReflection.descriptorSet,//IBL��specular�̋��ʔ���
                    iblSpecularBRDF.descriptorSet//IBL��specular��BRDF
                };

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mesh->descriptorInfo.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdPushConstants(commandBuffer, mesh->descriptorInfo.pLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObj), &constant);

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            drawMesh(node->children[i], model,commandBuffer);
        }
    }

    //�V���h�E�}�b�v�̍쐬
    void VulkanBase::calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer,OffScreenPass& pass)
    {
        Storage* storage = Storage::GetInstance();

        if (node->mesh)
        {
            Mesh* mesh = node->mesh;

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

    //�V���h�E�}�b�v�̗p��
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

    //�ʏ�̃����_�����O�̃R�}���h���L�^���Ă���
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

        Storage* storage = Storage::GetInstance();
        
        for (auto model:storage->getModels())
        {
            drawMesh((model)->getRootNode(),model,commandBuffer);

            if ((model)->hasColider())
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

        drawCubeMap(storage->getCubeMap()->getRootNode(), storage->getCubeMap(), commandBuffer);//�L���[�u�}�b�s���O�̕`��

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void VulkanBase::createSyncObjects() {//�`��̓��@�p�̕ϐ���p��
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    //�ʏ�̃����_�����O���J�n����
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
            updateUniformBuffers(model);//3D���f���̍��W�ϊ��s��Ȃǂ��X�V
        }

        updateUniformBuffer(storage->getPointLights(), storage->getPointLightsBuffer());//�|�C���g���C�g�̍��W�A�F�̍X�V
        updateUniformBuffer(storage->getDirectionalLights(), storage->getDirectionalLightsBuffer());//�f�B���N�V���i�����C�g�̌����A�F�̍X�V
        updateUniformBuffer(storage->getPointLights(), storage->getDirectionalLights());//�V���h�E�}�b�v�p�̍X�V

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

    //�V�F�[�_�̍쐬
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

    //gltf���f���̒��_�o�b�t�@�[�Ȃǂ̍쐬�A�t������R���C�_�[�̒��_�̃o�b�t�@�[���p��
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

    //gltf���f���̊e�m�[�h�Ƀp�C�v���C���Ƃ��̃��C�A�E�g��ݒ肷��
    void VulkanBase::createDescriptorInfo(VkPipelineLayout& pLayout, VkPipeline& pipeline, GltfNode* node, std::shared_ptr<Model> model)
    {
        if(node->mesh)
        {
            Mesh* mesh = node->mesh;
            for (int i = 0;i < mesh->primitives.size();i++)
            {
                mesh->descriptorInfo.pLayout = pLayout;
                mesh->descriptorInfo.pipeline = pipeline;
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            createDescriptorInfo(pLayout,pipeline,node->children[i], model);
        }
    }

    //�R���C�_�[�Ƀp�C�v���C���Ƃ��̃��C�A�E�g��ݒ肷��
    void VulkanBase::createDescriptorInfo(std::shared_ptr<Colider> colider)
    {
        DescriptorInfo& info = colider->getDescInfo();

        info.pLayout = modelDescriptor.coliderPipelineLayout;
        info.pipeline = modelDescriptor.coliderPipeline;
    }

    //gltf���f���̊e�m�[�h�Ƀp�C�v���C���Ƃ��̃��C�A�E�g��ݒ肷��
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

    //ShaderMaterial�̏����ݒ�ƃo�b�t�@�[�̗p��
    void VulkanBase::createShaderMaterialUBO(std::shared_ptr<Material> material)
    {
        material->setupShaderMaterial();

        createUniformBuffer(material);

        memcpy(material->sMaterialMappedBuffer.uniformBufferMapped, &material->shaderMaterial, sizeof(ShaderMaterial));
    }

    //�_�~�[�e�N�X�`���̃f�[�^�̍쐬
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

        if (descriptorSetCount > 100)
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

    void VulkanBase::prepareDescriptorSets()//�ʏ�̃����_�����O�ŕK�v��DescriptorSet�̃��C�A�E�g���쐬
    {
        //�܂�VkDescriptorSetLayout�����炩���ߍ���Ă���
        {
            //3D���f���̍��W�ϊ��p�̃��C�A�E�g
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
            //�}�e���A���p�̃��C�A�E�g
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
            //���C�g�p�̃��C�A�E�g
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
            //IBL�p�̃��C�A�E�g
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

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblDiffuse.mainPassLayout) != VK_SUCCESS)//IBL��diffuse
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblSpecularReflection.mainPassLayout) != VK_SUCCESS)//IBL��specular�̋��ʔ��˗p
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblSpecularBRDF.mainPassLayout) != VK_SUCCESS)//IBL��specular��BRDF�p
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
    }

    //���C�g��descriptorSet�֌W�̃f�[�^���쐬
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

        if (descriptorSetCount > 100)
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

    //�V���h�E�}�b�v�쐬�p��descriptorSet�֌W�̃f�[�^�̍쐬
    void VulkanBase::createDescriptorData_ShadowMap(std::vector<VkDescriptorSet>& descriptorSets,OffScreenPass& pass,VkDescriptorSetLayout& layout)
    {
        //���C�A�E�g��preapreDescriptor�ɂč쐬�ς�
        //�f�v�X�o�b�t�@����͂Ƃ���descriptorset�̃��C�A�E�g�̍쐬
        {
            //�V���h�E�}�b�v�p
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

            if (descriptorSetCount > 100)
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

    //�|�C���g���C�g��gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void VulkanBase::setPointLights(std::vector<std::shared_ptr<PointLight>> lights)
    {
        Storage* storage = Storage::GetInstance();

        createUniformBuffer(1, &storage->getPointLightsBuffer(), sizeof(PointLightUBO));

        createDescriptorData(storage->getPointLightsBuffer(), modelDescriptor.lightLayout,
            storage->getPointLightDescriptorSet(), sizeof(PointLightUBO), VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    //���s������gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void VulkanBase::setDirectionalLights(std::vector<std::shared_ptr<DirectionalLight>> lights)
    {
        Storage* storage = Storage::GetInstance();

        createUniformBuffer(1, &storage->getDirectionalLightsBuffer(), sizeof(DirectionalLightUBO));

        createDescriptorData(storage->getDirectionalLightsBuffer(), modelDescriptor.lightLayout,
            storage->getDirectionalLightDescriptorSet(), sizeof(DirectionalLightUBO), VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    //gltf���f���̎��}�e���A���p�̃f�[�^�̍쐬
    void VulkanBase::setGltfModelData(std::shared_ptr<GltfModel> gltfModel)
    {
        /*�e�N�X�`���֘A�̐ݒ����������*/
        createTextureImage(gltfModel);
        createTextureImageView(gltfModel);
        createTextureSampler(gltfModel);

        for (std::shared_ptr<Material> material : gltfModel->materials)
        {
            createShaderMaterialUBO(material);
            /*��������p�C�v���C���́A�����O���[�v�̃��f���ł͎g���܂킹��*/
            /*�f�B�X�N���v�^�Z�b�g�́A�e�N�X�`���f�[�^���قȂ�ꍇ�͎g���܂킹�Ȃ�*/

            /*�f�B�X�N���v�^�p�̃��������󂯂�*/
            allocateDescriptorSet(material);//�}�e���A������������ꍇ�G���[

            /*�f�B�X�N���v�^�Z�b�g�����*/
            createDescriptorSet(material, gltfModel);
        }
    }

    //�V���h�E�}�b�v�p���f�[�^��p�ӂ���A�����Ƃ��ăV�[����̃��C�g�̐������I�t�X�N���[�������_�����O���s��
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
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// �����_�[�p�X�J�n���ɐ[�x���N���A
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// �[�x��ǂݎ�邽�߁A�[�x�A�^�b�`�����g�̌��ʂ�ۑ����邱�Ƃ��d�v
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// �A�^�b�`�����g�̏������C�A�E�g�͋C�ɂ��Ȃ�
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// �����_�[�p�X�I�����ɁA�A�^�b�`�����g�̓V�F�[�_�[�ǂݎ��p�ɑJ�ڂ����

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 0;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// �����_�[�p�X���ɁA�A�^�b�`�����g�͐[�x/�X�e���V���Ƃ��Ďg�p�����

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;													// �J���[�A�^�b�`�����g�Ȃ�
        subpass.pDepthStencilAttachment = &depthReference;									// �[�x�A�^�b�`�����g�ւ̎Q��

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//�ŏ���
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//��
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

            if (descriptorSetCount > 100)
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

        createDescriptorData_ShadowMap(shadowMapData.descriptorSets,shadowMapData.passData,shadowMapData.layout);//�f�v�X�o�b�t�@���e�N�X�`���Ƃ��ė��p���邽�߂�descriptorSet
    }

    //�e�탉�C�g�̃o�b�t�@�Ȃǂ̍쐬
    void VulkanBase::setLightData(std::vector<std::shared_ptr<PointLight>> pointLights, std::vector<std::shared_ptr<DirectionalLight>> dirLights)
    {
        setPointLights(pointLights);
        setDirectionalLights(dirLights);
    }

    //�e��o�b�t�@�[�Ȃǂ̃f�[�^�̍쐬 lua���s��ɍs��
    void VulkanBase::prepareDescriptorData(int lightCount)
    {
        prepareShadowMapping(lightCount);//�V���h�E�}�b�v�̗p��

        /*�O���t�B�b�N�X�p�C�v���C�������*/
        createGraphicsPipeline("shaders/vert.spv", "shaders/frag.spv",
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, modelDescriptor.layout, modelDescriptor.texturePipelineLayout, modelDescriptor.texturePipeline);//���ʂ�3D���f���\���p

        createGraphicsPipeline("shaders/line.vert.spv", "shaders/line.frag.spv",
            VK_PRIMITIVE_TOPOLOGY_LINE_LIST, modelDescriptor.layout, modelDescriptor.coliderPipelineLayout, modelDescriptor.coliderPipeline);//�R���C�_�[�\���p
    }

    //Model�N���X�̎��o�b�t�@�[�̍쐬
    void VulkanBase::setModelData(std::shared_ptr<Model> model)
    {
        /*���_�A�C���f�b�N�X�o�b�t�@�[����������*/
        createMeshesData(model);

        /*UnifomBuffer����������*/
        createUniformBuffers(model);

        /*��������p�C�v���C���́A�����O���[�v�̃��f���ł͎g���܂킹��*/
        /*�f�B�X�N���v�^�Z�b�g�́A�e�N�X�`���f�[�^���قȂ�ꍇ�͎g���܂킹�Ȃ�*/
        createDescriptorInfos(modelDescriptor.texturePipelineLayout, modelDescriptor.texturePipeline,model);

        /*�f�B�X�N���v�^�p�̃��������󂯂�*/
        allocateDescriptorSets(modelDescriptor.layout,model);//�}�e���A������������ꍇ�G���[

        /*�f�B�X�N���v�^�Z�b�g�����*/
        createDescriptorSets(model);

        model->getGltfModel()->setup = true;
    }

    //�L���[�u�}�b�v�̍쐬
    void VulkanBase::prepareCubemapTextures()
    {
        //hdriTexture�̃f�[�^�����
        {
            createTextureImage(*cubemapData.srcHdriTexture, Storage::GetInstance()->getCubemapImage());
            createTextureImageView(*cubemapData.srcHdriTexture);
            createTextureSampler(cubemapData.srcHdriTexture);
        }

        cubemapData.setFrameCount(CUBEMAP_FACE_COUNT);

        //�L���[�u�}�b�s���O�p�Ƀ����_�����O�C���[�W�𐳕��`�ɂ��Ă���
        std::shared_ptr<ImageData> cubemapImage = Storage::GetInstance()->getCubemapImage();
        int renderSize = std::min(cubemapImage->getWidth(), cubemapImage->getHeight());

        cubemapData.passData.width = renderSize;
        cubemapData.passData.height = renderSize;

        //�����_�����O�̌��ʂ̏o�͗p�̃o�b�t�@�̍쐬
        for (auto& attachment : cubemapData.passData.imageAttachment)
        {
            createImage(renderSize,renderSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.memory);
            attachment.view = createImageView(attachment.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, cubemapData.passData.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// �����_�[�p�X�J�n���ɐ[�x���N���A
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// �A�^�b�`�����g�̏����L�^
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// �A�^�b�`�����g�̏������C�A�E�g�͋C�ɂ��Ȃ�
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// �����_�[�p�X�I�����ɁA�A�^�b�`�����g�̓V�F�[�_�[�ǂݎ��p�ɑJ�ڂ����

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;									

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//�ŏ���
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = 0;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//��
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

            if (descriptorSetCount > 100)
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

    void VulkanBase::prepareIBL(std::string vertShaderPath, std::string fragShaderPath,OffScreenPass& passData)
    {
        //�����_�����O�̌��ʂ̏o�͗p�̃o�b�t�@�̍쐬
        for (auto& attachment : passData.imageAttachment)
        {
            createImage(passData.width, passData.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.memory);
            attachment.view = createImageView(attachment.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, passData.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// �����_�[�p�X�J�n���ɐ[�x���N���A
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// �A�^�b�`�����g�̏����L�^
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// �A�^�b�`�����g�̏������C�A�E�g�͋C�ɂ��Ȃ�
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// �����_�[�p�X�I�����ɁA�A�^�b�`�����g�̓V�F�[�_�[�ǂݎ��p�ɑJ�ڂ����

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//�ŏ���
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = 0;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//��
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

            if (descriptorSetCount > 100)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            //�L���[�u�}�b�s���O�Ŏg���s��̃o�b�t�@�𗬗p����
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = cubemapData.mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            //6�̃��C���[�����摜���Q�Ƃ���悤�ɂ���
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

        //�����Ŏw�肳���V�F�[�_��ibl�̃e�N�X�`���쐬�p
        createCalcIBLMapPipeline(vertShaderPath, fragShaderPath, passData.layout, passData.pipelineLayout, 1,&passData.pipeline, 1,&passData.renderPass);
    }

    //specular��IBL�̍쐬�̂��߁A�����_�[�p�X�𕡐��쐬
    void VulkanBase::prepareIBL(IBLSpecularReflection& iblSpecular)
    {
        //�����_�����O�̌��ʂ̏o�͗p�̃o�b�t�@�̍쐬
        for (int i = 0; i < iblSpecular.imageAttachment.size(); i++)
        {
            createImage(iblSpecular.mipmapLevelSize[i % iblSpecular.mipmapLevel], iblSpecular.mipmapLevelSize[i % iblSpecular.mipmapLevel], 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, iblSpecular.imageAttachment[i].image, iblSpecular.imageAttachment[i].memory);
            iblSpecular.imageAttachment[i].view =
                createImageView(iblSpecular.imageAttachment[i].image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, iblSpecular.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// �����_�[�p�X�J�n���ɐ[�x���N���A
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// �A�^�b�`�����g�̏����L�^
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// �A�^�b�`�����g�̏������C�A�E�g�͋C�ɂ��Ȃ�
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// �����_�[�p�X�I�����ɁA�A�^�b�`�����g�̓V�F�[�_�[�ǂݎ��p�ɑJ�ڂ����

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//�ŏ���
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = 0;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//��
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

        //�����̃����_�[�p�X�ɑ΂��āA�t���[���o�b�t�@�̍쐬
        //��̍s��ɑ΂��āA�����̉𑜓x�̃p�X������
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

            if (descriptorSetCount > 100)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            //�L���[�u�}�b�s���O�Ŏg���s��̃o�b�t�@�𗬗p����
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = cubemapData.mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            //6�̃��C���[�����摜���Q�Ƃ���悤�ɂ���
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
     
        //�����Ŏw�肳���V�F�[�_��ibl�̃e�N�X�`���쐬�p
        createCalcIBLMapPipeline(iblSpecular.vertShaderPath, iblSpecular.fragShaderPath, iblSpecular.prePassLayout,iblSpecular.pipelineLayout
            , static_cast<uint32_t>(iblSpecular.pipeline.size()), iblSpecular.pipeline.data(), static_cast<uint32_t>(iblSpecular.renderPass.size()), iblSpecular.renderPass.data());
    }

    //specular��IBL�̍쐬�̂��߁A�����_�[�p�X�𕡐��쐬
    void VulkanBase::prepareIBL(IBLSpecularBRDF& iblSpecular)
    {
        //IBL��Specular��BRDF�̏�ݍ��݂ł́A�I�t�X�N���[�������_�����O�ł͈�x�����s��Ȃ��̂ŁA
        //�����̔z��̃C���f�b�N�X�̈�ڂ̗v�f�����g��Ȃ�
        const int index = 0;

        //�����_�����O�̌��ʂ̏o�͗p�̃o�b�t�@�̍쐬
        createImage(iblSpecular.passData.width, iblSpecular.passData.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, iblSpecular.passData.imageAttachment[index].image, iblSpecular.passData.imageAttachment[index].memory);
        iblSpecular.passData.imageAttachment[index].view =
            createImageView(iblSpecular.passData.imageAttachment[index].image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);

        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, iblSpecular.passData.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// �����_�[�p�X�J�n���ɐ[�x���N���A
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// �A�^�b�`�����g�̏����L�^
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// �A�^�b�`�����g�̏������C�A�E�g�͋C�ɂ��Ȃ�
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// �����_�[�p�X�I�����ɁA�A�^�b�`�����g�̓V�F�[�_�[�ǂݎ��p�ɑJ�ڂ����

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//�ŏ���
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = 0;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;//��
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

        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &iblSpecular.passData.renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create renderpass");
        }

        //�����̃����_�[�p�X�ɑ΂��āA�t���[���o�b�t�@�̍쐬
        //��̍s��ɑ΂��āA�����̉𑜓x�̃p�X������
        VkFramebufferCreateInfo fbufCreateInfo{};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = iblSpecular.passData.renderPass;
        fbufCreateInfo.attachmentCount = 1;
        fbufCreateInfo.pAttachments = &iblSpecular.passData.imageAttachment[index].view;
        fbufCreateInfo.width = iblSpecular.passData.width;
        fbufCreateInfo.height = iblSpecular.passData.height;

        fbufCreateInfo.layers = 1;

        if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &iblSpecular.passData.frameBuffer[index]))
        {
            throw std::runtime_error("failed create frame buffer");
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

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblSpecular.passData.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
        allocInfo.pSetLayouts = &iblSpecular.passData.layout;

        if (vkAllocateDescriptorSets(device, &allocInfo, &iblSpecular.passData.descriptorSets[index]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        if (descriptorSetCount > 100)
        {
            throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
        }
        descriptorSetCount++;

        //�L���[�u�}�b�s���O�Ŏg���s��̃o�b�t�@�𗬗p����
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = cubemapData.mappedBuffers[i].uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatricesUBO);

        //6�̃��C���[�����摜���Q�Ƃ���悤�ɂ���
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = cubemapData.multiTexture->view;
        info.sampler = cubemapData.multiTexture->sampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites(2);
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = iblSpecular.passData.descriptorSets[index];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = iblSpecular.passData.descriptorSets[index];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &info;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        //�����Ŏw�肳���V�F�[�_��ibl�̃e�N�X�`���쐬�p
        createCalcIBLMapPipeline(iblSpecular.vertShaderPath, iblSpecular.fragShaderPath, iblSpecular.passData.layout, iblSpecular.passData.pipelineLayout
            , 1, &iblSpecular.passData.pipeline, 1, &iblSpecular.passData.renderPass);
    }

    //6���̃e�N�X�`�����쐬���āA�L���[�u�}�b�v���쐬
    void VulkanBase::createSamplerCube2D(OffScreenPass& passData,std::vector<MappedBuffer>& mappedBuffers)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        Storage* storage = Storage::GetInstance();
        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            {//�J�����𗧕��̂̓�������A6�̖ʂ��f���o���悤�ɁA�r���[�s����X�V���Ă���

                MatricesUBO ubo;
                ubo.proj = glm::perspective(glm::radians(90.0f),1.0f, 0.1f, 100.0f);
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

            drawSamplerCube(storage->getgltfModel(GLTFOBJECT::CUBEMAP)->getRootNode(), storage->getCubeMap()
                , passData.width, passData.width, commandBuffers[0], i, passData.descriptorSets
                ,passData.pipelineLayout,passData.pipeline);

            vkCmdEndRenderPass(commandBuffers[0]);
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

    //specular�p��SamplerCube�����֐�
    void VulkanBase::createSamplerCube2D(IBLSpecularReflection& iblSpecular, std::vector<MappedBuffer>& mappedBuffers)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        SpecularPushConstant constant;

        Storage* storage = Storage::GetInstance();
        //6�̖ʂ������_�����O����
        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            {//�J�����𗧕��̂̓�������A6�̖ʂ��f���o���悤�ɁA�r���[�s����X�V���Ă���

                MatricesUBO ubo;
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

            //��̖ʂɑ΂��āA������ނ̉𑜓x�Ń����_�����O����
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

                //�~�b�v�}�b�v���x���ɉ����āAroughness�Ƃ��ăV�F�[�_�ɒl��n���āABRDF�̒l�𒲐�����
                constant.roughness = static_cast<float>(j) / static_cast<float>(iblSpecular.mipmapLevel);
                vkCmdPushConstants(commandBuffers[0], iblSpecular.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SpecularPushConstant), &constant);

                drawSamplerCube(storage->getgltfModel(GLTFOBJECT::CUBEMAP)->getRootNode(), storage->getCubeMap()
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

    //specular�p��SamplerCube�����֐�
    void VulkanBase::createLUT(IBLSpecularBRDF& iblSpecular, MappedBuffer& mappedBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        SpecularPushConstant constant;

        Storage* storage = Storage::GetInstance();

        //��̖ʂ̂݃����_�����O����
        MatricesUBO ubo;
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

        drawSamplerCube(storage->getgltfModel(GLTFOBJECT::CUBEMAP)->getRootNode(), storage->getCubeMap()
            , iblSpecular.passData.width, iblSpecular.passData.height, commandBuffers[0], 0/*��̖ʂ��������_�����O���Ȃ��̂�0�ŌŒ�*/, iblSpecular.passData.descriptorSets
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

    //�L���[�u�}�b�v���쐬���邽�߂̉摜�������_�����O
    void VulkanBase::drawSamplerCube(GltfNode* node, std::shared_ptr<Model> model
        ,uint32_t width,uint32_t height, VkCommandBuffer& commandBuffer, int index,std::vector<VkDescriptorSet>& descriptorSets
        ,VkPipelineLayout& pipelineLayout,VkPipeline& pipeline)
    {
        if (node->mesh)
        {
            Mesh* mesh = node->mesh;

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

    void VulkanBase::createMultiLayerTexture(TextureData* dstTextureData, uint32_t layerCount,uint32_t width,uint32_t height,uint32_t mipLevel)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevel;
        imageInfo.arrayLayers = layerCount;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if (vkCreateImage(device, &imageInfo, nullptr, &dstTextureData->image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
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

    //�L���[�u�}�b�v�̍쐬
    void VulkanBase::setCubeMapModel(std::shared_ptr<Model> cubemap)
    {
        prepareCubemapTextures();//�L���[�u�}�b�s���O�̗p��

        /*���_�A�C���f�b�N�X�o�b�t�@�[����������*/
        createMeshesData(cubemap);

        /*UnifomBuffer����������*/
        createUniformBuffers(cubemap);

        //6���̃e�N�X�`�����쐬����
        createSamplerCube2D(cubemapData.passData,cubemapData.mappedBuffers);

        /*�X�J�C�{�b�N�X�p�̃e�N�X�`����p�ӂ���*/
        {
            {
                //�X�J�C�{�b�N�X�p�Ƀ��C���[�̐���6��
                std::shared_ptr<ImageData> image = Storage::GetInstance()->getCubemapImage();
                int cubemapImageSize = std::min(image->getWidth(), image->getHeight());

                //�t���[���o�b�t�@����L���[�u�}�b�v�̃����_�����O���ʂ��擾��
                //�t���[���o�b�t�@�̉摜�̃��C�A�E�g�𑗐M�p�ɂ���
                {
                    for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
                    {
                        transitionImageLayout(cubemapData.passData.imageAttachment[i].image,
                            VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);
                    }
                }

                cubemapData.multiTexture->mipLevel = calcMipMapLevel(cubemapImageSize, cubemapImageSize);

                {//6�̃��C���[�����e�N�X�`���f�[�^���쐬
                    createMultiLayerTexture(cubemapData.multiTexture, CUBEMAP_FACE_COUNT, cubemapImageSize, cubemapImageSize, cubemapData.multiTexture->mipLevel);

                    //�摜�̂��ׂẴ��C���[���ړ�������
                    transitionImageLayout(cubemapData.multiTexture->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cubemapData.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);

                    std::vector<VkImage> srcImages(CUBEMAP_FACE_COUNT);
                    for (uint32_t i = 0;i < cubemapData.passData.imageAttachment.size();i++)
                    {
                        srcImages[i] = cubemapData.passData.imageAttachment[i].image;
                    }
                    copyImageToMultiLayerImage(srcImages.data(), static_cast<uint32_t>(srcImages.size())
                        , cubemapData.passData.width, cubemapData.passData.height, cubemapData.multiTexture->image);

                    generateMipmaps(cubemapData.multiTexture->image, VK_FORMAT_R8G8B8A8_UNORM, cubemapImageSize, cubemapImageSize, cubemapData.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);
                }
            }

            //�r���[���e�N�X�`���L���[�u�ɐݒ�
            cubemapData.multiTexture->view = createImageView(cubemapData.multiTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT
                , cubemapData.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);

            createTextureSampler(cubemapData.multiTexture);
        }

        {
            //�L���[�u�}�b�s���O�𔽉f���邽�߂�descriptorSet�̃��C�A�E�g���쐬

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

        /*��������p�C�v���C���́A�����O���[�v�̃��f���ł͎g���܂킹��*/
        /*�f�B�X�N���v�^�Z�b�g�́A�e�N�X�`���f�[�^���قȂ�ꍇ�͎g���܂킹�Ȃ�*/
        createDescriptorInfos(cubemapData.pipelineLayout, cubemapData.pipeline, cubemap);

        //cubemap�̃e�N�X�`��������descriptorSet�̃������[���m��
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

            if (descriptorSetCount > 100)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;
        }

        //SamplerCube�p��descriptorSet�̍쐬
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

        /*�f�B�X�N���v�^�p�̃��������󂯂�*/
        allocateDescriptorSets(modelDescriptor.layout, cubemap);//�}�e���A������������ꍇ�G���[

        /*MatricesUBO��descriptorSet�����*/
        createDescriptorSet_CubeMap(cubemap->getRootNode(), cubemap);

        //IBL���쐬����
        createIBL();

        cubemap->getGltfModel()->setup = true;
    }

    //IBL���쐬����
    void VulkanBase::createIBL()
    {
        //diffuse
        createIBLDiffuse();
        //specular
        createIBLSpecular();
    }

    //IBL��diffuse�e�N�X�`�����쐬����
    void VulkanBase::createIBLDiffuse()
    {
        //���O�v�Z�p�̃V�F�[�_�̐ݒ�
        iblDiffuse.vertShaderPath = "shaders/calcIBL.vert.spv";
        iblDiffuse.fragShaderPath = "shaders/calcDiffuse.frag.spv";

        //�I�t�X�N���[�������_�����O�p�̃t���[�������쐬����
        iblDiffuse.passData.setFrameCount(CUBEMAP_FACE_COUNT);
        //�e�N�X�`���̃T�C�Y�ݒ�
        iblDiffuse.setRenderSize(IBL_MAP_SIZE);

        //�~�b�v�}�b�v���x���̌v�Z
        iblDiffuse.setMipmapLevel(calcMipMapLevel(iblDiffuse.size, iblDiffuse.size));

        //diffuse�p�e�N�X�`����6��̃I�t�X�N���[�������_�����O�̏������s��
        prepareIBL(iblDiffuse.vertShaderPath, iblDiffuse.fragShaderPath, iblDiffuse.passData);

        //SamperCube���쐬����
        createSamplerCube2D(iblDiffuse.passData, cubemapData.mappedBuffers);

        //6�̎��_�̉摜���܂Ƃ߂āASamplerCube���쐬����
        createCubeMapTextureFromImages(iblDiffuse.size, iblDiffuse.multiLayerTexture, iblDiffuse.passData.imageAttachment);

        //�ʏ탌���_�����O���Ɏg��DescriptorSet�֘A�̃f�[�^���쐬
        createIBLDescriptor(iblDiffuse.multiLayerTexture, iblDiffuse.mainPassLayout, iblDiffuse.descriptorSet);
    }

    //IBL��specular�e�N�X�`�����쐬����
    void VulkanBase::createIBLSpecular()
    {
        //specular�̋��ʔ��ˍ��̃}�b�v���쐬
        createIBLSpecularReflection();
        //specular��BRDF�̃}�b�v���쐬
        createIBLSpecularBRDF();
    }

    void VulkanBase::createIBLSpecularReflection()
    {
        //���O�v�Z�p�̃V�F�[�_��ݒ�
        iblSpecularReflection.vertShaderPath = "shaders/calcIBL.vert.spv";
        iblSpecularReflection.fragShaderPath = "shaders/calcSpecularReflection.frag.spv";

        //�e�N�X�`���̃T�C�Y�̐ݒ�
        iblSpecularReflection.setRenderSize(IBL_MAP_SIZE);

        //specular�p�e�N�X�`����6��̃I�t�X�N���[�������_�����O�̏������s��
        prepareIBL(iblSpecularReflection);

        //6�̎��_����̗����̂̓����������_�����O���āA6�̎��_���ꂼ��̉摜���~�b�v�}�b�v�̐������쐬
        createSamplerCube2D(iblSpecularReflection, cubemapData.mappedBuffers);

        //6�̎��_�̉摜���܂Ƃ߂āASamplerCube���쐬����
        createCubeMapTextureFromImages(iblSpecularReflection.size, iblSpecularReflection.multiLayerTexture
            , iblSpecularReflection.imageAttachment, iblSpecularReflection.mipmapLevelSize);

        //�ʏ탌���_�����O���Ɏg��DescriptorSet�֘A�̃f�[�^���쐬
        createIBLDescriptor(iblSpecularReflection.multiLayerTexture, iblSpecularReflection.mainPassLayout, iblSpecularReflection.descriptorSet);
    }

    //specular��BRDF���ɂ��Ẵ}�b�v���쐬
    void VulkanBase::createIBLSpecularBRDF()
    {
        //���O�v�Z�p�̃V�F�[�_��ݒ�
        iblSpecularBRDF.vertShaderPath = "shaders/calcIBL.vert.spv";
        iblSpecularBRDF.fragShaderPath = "shaders/calcSpecularBRDF.frag.spv";

        //�e�N�X�`���̃T�C�Y�̐ݒ�
        iblSpecularBRDF.setRenderSize(IBL_MAP_SIZE);

        //IBL��speuclar��BRDF�ł́A�����̂̈�ʂ̂݃����_�����O����΂���
        iblSpecularBRDF.passData.setFrameCount(1);

        //specular�p�e�N�X�`����6��̃I�t�X�N���[�������_�����O�̏������s��
        prepareIBL(iblSpecularBRDF.vertShaderPath, iblSpecularBRDF.fragShaderPath, iblSpecularBRDF.passData);

        //��̎��_�݂̂��痧���̖̂ʂ������_�����O���āA2D��LUT���쐬����
        createLUT(iblSpecularBRDF, cubemapData.mappedBuffers[0]);

        //�����_�����O�����摜���V�F�[�_�œǂݎ�邽�߂̃e�N�X�`���ɕϊ�����
        transitionImageLayout(iblSpecularBRDF.passData.imageAttachment[0].image, VK_FORMAT_R8G8B8A8_UNORM
            , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

        //�����̂̈�̖ʂ݂̂̃����_�����O���ʂ�LUT�Ƃ��ė��p
        //�ʏ탌���_�����O���Ɏg��DescriptorSet�֘A�̃f�[�^���쐬
        createIBLDescriptor(iblSpecularBRDF.passData, iblSpecularBRDF.mainPassLayout, iblSpecularBRDF.descriptorSet);
    }

    //6�̉摜����̉摜�ɂ܂Ƃ߂āASamplerCube�����
    void VulkanBase::createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment)
    {
        //6�̃��C���[�����摜�����
        createMultiLayerTexture(multiLayerTexture, CUBEMAP_FACE_COUNT, texSize, texSize, multiLayerTexture->mipLevel);

        //�����_�����O����6���̉摜��]���p�̃��C�A�E�g�ɕύX����
        for (int i = 0; i < imageAttachment.size(); i++)
        {
            transitionImageLayout(imageAttachment[i].image,
                VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);
        }

        //SamplerCube�̉摜�̂��ׂẴ��C���[���ړ�������
        transitionImageLayout(multiLayerTexture->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);


        //���܂ō쐬�����𑜓x�̈Ⴄ�摜���A����̎��_�̉摜�̃~�b�v�}�b�v�Ƃ��Ċi�[����
        //�������_�̃����_�����O�摜���W�߂āA������~�b�v�}�b�v�Ƃ��Ă܂Ƃ߂Ă���
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
            {
                //���炩���߃����_�����O���Ă������e�ʂ�specular�̃e�N�X�`���̑f�ނ�
                //�����ŁA6�̃��C���[�������A�����̃~�b�v�}�b�v�����摜�f�[�^�ɂЂƂ܂Ƃ߂ɂ���

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

        //�~�b�v�}�b�v�̍쐬
        generateMipmaps(multiLayerTexture->image, VK_FORMAT_R8G8B8A8_UNORM,
            texSize, texSize, multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        //�r���[���e�N�X�`���L���[�u�ɐݒ�
        multiLayerTexture->view = createImageView(multiLayerTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        createTextureSampler(multiLayerTexture);
    }

    //6�̉摜����̉摜�ɂ܂Ƃ߂āASamplerCube�����
    void VulkanBase::createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, std::vector<uint32_t>& mipmapLevelSize)
    {
        //6�̃��C���[�����摜�����
        createMultiLayerTexture(multiLayerTexture, CUBEMAP_FACE_COUNT, texSize, texSize, multiLayerTexture->mipLevel);

        //�����_�����O����6���̉摜��]���p�̃��C�A�E�g�ɕύX����
        for (int i = 0; i < imageAttachment.size(); i++)
        {
            transitionImageLayout(imageAttachment[i].image,
                VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);
        }

        //SamplerCube�̉摜�̂��ׂẴ��C���[���ړ�������
        transitionImageLayout(multiLayerTexture->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);


        //���܂ō쐬�����𑜓x�̈Ⴄ�摜���A����̎��_�̉摜�̃~�b�v�}�b�v�Ƃ��Ċi�[����
        //�������_�̃����_�����O�摜���W�߂āA������~�b�v�}�b�v�Ƃ��Ă܂Ƃ߂Ă���
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
            {
                //���炩���߃����_�����O���Ă������e�ʂ�specular�̃e�N�X�`���̑f�ނ�
                //�����ŁA6�̃��C���[�������A�����̃~�b�v�}�b�v�����摜�f�[�^�ɂЂƂ܂Ƃ߂ɂ���

                for (int j = 0; j < multiLayerTexture->mipLevel; j++)
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

        //�~�b�v�}�b�v�̍쐬
        generateMipmaps(multiLayerTexture->image, VK_FORMAT_R8G8B8A8_UNORM,
            texSize, texSize, multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        //�r���[���e�N�X�`���L���[�u�ɐݒ�
        multiLayerTexture->view = createImageView(multiLayerTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        createTextureSampler(multiLayerTexture);
    }

    //IBL�p��DescriptorSet�̗p��
    void VulkanBase::createIBLDescriptor(TextureData* samplerCube, VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet)
    {
        //DescriptorSetLayout�̍쐬
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

        {//DescriptorSet�̎��Ԃ��쐬
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > 100)
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

    //IBL�p��DescriptorSet�̗p��(LUT�p)
    void VulkanBase::createIBLDescriptor(OffScreenPass& passData, VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet)
    {
        //DescriptorSetLayout�̍쐬
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

        {//DescriptorSet�̎��Ԃ��쐬
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > 100)
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

    //�����_�����O�̊J�n
    void VulkanBase::render()
    {
        descriptorSetCount = 0;

        /*�`��*/
        drawFrame();
    }