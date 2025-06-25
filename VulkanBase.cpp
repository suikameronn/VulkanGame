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
        createCommandBuffers();
        createSyncObjects();
        createDescriptorPool();
        createEmptyImage();

        createDescriptorSetLayout();//VkDescriptorSetLayout�̗p��
        prepareUIRendering();//ui�`��̗p��
        createPipelines();//�p�C�v���C�������炩���ߍ쐬

        //���C�L���X�g�̃Z�b�g�A�b�v
        setupRaycast();
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
        modelDescriptor.destroy(device);
        uiRender.destroy(device,multiThreadCommandPool);

        //���C�L���X�g�̃f�[�^�̔j��
        raycast.destroy(device, commandPool);

        Storage* storage = Storage::GetInstance();

        for (auto& gltfModel:storage->getgltfModel())//gltf���f���̃o�b�t�@�[�Ȃǂ̍폜
        {
            gltfModel.second->cleanUpVulkan(device);
        }

        for (auto& image : storage->getImageData())
        {
            image.second->cleanUpVulkan(device);
        }

        //storage->getLoadUI()->cleanupVulkan();

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
            vkDestroyFence(device, multiThreadFences[i], nullptr);
        }

        vkDestroyDescriptorSetLayout(device, shadowmapLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, iblLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, fontLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, textLayout, nullptr);

        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyCommandPool(device, multiThreadCommandPool, nullptr);

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

        std::array<float, 2> queuePrioritys = { 1.0f,0.0f };
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;//�}���`�X���b�h�p�̃L���[���m�ۂ���
            queueCreateInfo.pQueuePriorities = queuePrioritys.data();
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        deviceFeatures.shaderInt64 = VK_TRUE;

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

        //vkGetDeviceQueue(device, indices.graphicsFamily.value(), 1, &multiThreadGraphicQueue);//�C���f�b�N�X���ڂɐݒ�
        //vkGetDeviceQueue(device, indices.presentFamily.value(), 1, &multiThreadPresentQueue);
    }

    //�X���b�v�`�F�[���̍쐬
    void VulkanBase::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

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

        defferedDestruct.setSwapChainImageCount(imageCount);
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

            std::cout << offsetof(RaycastReturn, pointer) << std::endl;
            std::cout << offsetof(RaycastReturn, distance) << std::endl;

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
        rasterizer.lineWidth = 1.0f;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
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
        pushConstant.size = sizeof(FragmentParam);
        pushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayout> layouts(8);
        layouts[0] = layout;//MVP�s��ƃA�j���[�V�����s��
        layouts[1] = modelDescriptor.materialLayout;//�}�e���A��
        layouts[2] = modelDescriptor.lightLayout;//�|�C���g���C�g
        layouts[3] = modelDescriptor.lightLayout;//���s����
        layouts[4] = shadowmapLayout;//�V���h�E�}�b�v
        layouts[5] = iblLayout;//IBL��diffuse�p
        layouts[6] = iblLayout;//IBL��specular�̋��ʔ��˗p
        layouts[7] = iblLayout;//IBL��specular��BRDF�p

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

        //�}���`�X���b�h�p���쐬����
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &multiThreadCommandPool) != VK_SUCCESS)
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
    void VulkanBase::createTextureImage(std::shared_ptr<ImageData> image,VkFormat format)
    {
        TextureData* texture = image->getTexture();

        VkDeviceSize bufferSize = image->getWidth() * image->getHeight() * image->getTexChannels() * image->getPixelPerByte();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, image->getPixelsData(), bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        texture->mipLevel = calcMipMapLevel(image->getWidth(),image->getHeight());
        createImage(image->getWidth(), image->getHeight(), texture->mipLevel, VK_SAMPLE_COUNT_1_BIT
            , format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->image, texture->memory);

        transitionImageLayout(texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->mipLevel,1);
        copyBufferToImage(stagingBuffer, texture->image, static_cast<uint32_t>(image->getWidth()), static_cast<uint32_t>(image->getHeight()),1);

        generateMipmaps(texture->image, format, image->getWidth(), image->getHeight(), texture->mipLevel,1);

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
    void VulkanBase::createTextureImage(std::shared_ptr<GltfModel> gltfModel,VkFormat format)
    {
        if (gltfModel->imageDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->imageDatas.size(); i++)
        {
            std::shared_ptr<ImageData> imageData = gltfModel->imageDatas[i];
            TextureData* textureData = imageData->getTexture();

            VkDeviceSize imageSize = imageData->getWidth() * imageData->getHeight() * imageData->getTexChannels() * imageData->getPixelPerByte();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, imageData->getPixelsData(), imageSize);
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
    void VulkanBase::createTextureImageView(TextureData* textureData,VkFormat format)
    {
        textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
    }

    //�_�~�[�e�N�X�`���p
    void VulkanBase::createTextureImageView()
    {
        TextureData* textureData = emptyImage.emptyTex;
        textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
    }

    //gltf���f���̃e�N�X�`���p�̃r���[�̍쐬
    void VulkanBase::createTextureImageView(std::shared_ptr<GltfModel> gltfModel,VkFormat format) 
    {
        if (gltfModel->imageDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->imageDatas.size(); i++)
        {
            TextureData* textureData = gltfModel->imageDatas[i]->getTexture();
            textureData->view = createImageView(textureData->image, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT, textureData->mipLevel,1);
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
        if (gltfModel->imageDatas.size() == 0)
        {
            return;
        }

        for (int i = 0; i < gltfModel->imageDatas.size(); i++)
        {
            TextureData* textureData = gltfModel->imageDatas[i]->getTexture();

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
        //���C�A�E�g�������_�����O�̏o�͐�ɂ���
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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
        //�J���[�A�^�b�`�����g�p�̃��C�A�E�g��\���p�̃��C�A�E�g�ɂ���
        else if(oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = 0;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            //�p�C�v���C���̏����̂��ׂĂ��I�������Ƃ�
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        //�\���p�̃��C�A�E�g���J���[�A�^�b�`�����g�p�̃��C�A�E�g�ɂ���
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

    //ui�̃e�N�X�`�����쐬����
    void VulkanBase::createTexture(std::shared_ptr<ImageData> image,VkFormat format)
    {
        //gpu��ɉ摜�f�[�^��W�J
        createTextureImage(image, format);

        //���̃e�N�X�`���̃r���[���쐬
        createTextureImageView(image->getTexture(), format);

        //�T���v���[�̍쐬
        createTextureSampler(image->getTexture());
    }

    //Model�N���X�p�̒��_�o�b�t�@�[���쐬����
    void VulkanBase::createVertexBuffer(GltfNode* node,std::shared_ptr<GltfModel> gltfModel)
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

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                gltfModel->getPointBuffer()[mesh->meshIndex].vertBuffer, gltfModel->getPointBuffer()[mesh->meshIndex].vertHandler);

            //vertexBuffer�z��ɃR�s�[���Ă���(vector�^)
            copyBuffer(stagingBuffer, gltfModel->getPointBuffer()[mesh->meshIndex].vertBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        for (size_t i = 0; i < node->children.size(); i++)
        {
            createVertexBuffer(node->children[i], gltfModel);
        }
    }

    //UI�N���X�p�̒��_�o�b�t�@�[���쐬����
    void VulkanBase::createVertexBuffer(std::shared_ptr<UI> ui)
    {
        const int pointBufferNum = ui->getPointBufferSize();

        VkDeviceSize bufferSize = sizeof(Vertex2D) * ui->getVerticesSize();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        for (int i = 0; i < pointBufferNum; i++)
        {
            BufferObject& buffer = ui->getPointBuffer()[i];

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, ui->getVertices(), (size_t)bufferSize);
            vkUnmapMemory(device, stagingBufferMemory);

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                buffer.vertBuffer, buffer.vertHandler);

            //vertexBuffer�z��ɃR�s�[���Ă���(vector�^)
            copyBuffer(stagingBuffer, buffer.vertBuffer, bufferSize);
        }

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
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
    void VulkanBase::createIndexBuffer(GltfNode* node, std::shared_ptr<GltfModel> gltfModel)
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

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                gltfModel->getPointBuffer()[mesh->meshIndex].indeBuffer, gltfModel->getPointBuffer()[mesh->meshIndex].indeHandler);

            copyBuffer(stagingBuffer, gltfModel->getPointBuffer()[mesh->meshIndex].indeBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        for (size_t i = 0; i < node->children.size(); i++)
        {
            createIndexBuffer(node->children[i], gltfModel);
        }
    }

    //UI�p�̃C���f�b�N�X�o�b�t�@�[�̍쐬
    void VulkanBase::createIndexBuffer(std::shared_ptr<UI> ui) 
    {
        VkDeviceSize bufferSize = sizeof(uint32_t) * ui->getIndicesSize();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        const int pointBufferNum = ui->getPointBufferSize();
        for (int i = 0; i < pointBufferNum; i++)
        {
            BufferObject& buffer = ui->getPointBuffer()[i];

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, ui->getIndices(), (size_t)bufferSize);
            vkUnmapMemory(device, stagingBufferMemory);

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                buffer.indeBuffer, buffer.indeHandler);

            copyBuffer(stagingBuffer, buffer.indeBuffer, bufferSize);
        }

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //�R���C�_�[�p�̃C���f�b�N�X�o�b�t�@�[���쐬
    void VulkanBase::createIndexBuffer(std::shared_ptr<Colider> colider)
    {
        VkDeviceSize bufferSize = sizeof(*colider->getDrawColiderIndices()) * colider->getDrawColiderIndicesSize();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, colider->getDrawColiderIndices(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colider->getPointBufferData()->indeBuffer, colider->getPointBufferData()->indeHandler);

        copyBuffer(stagingBuffer, colider->getPointBufferData()->indeBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //���j�t�H�[���o�b�t�@�̍쐬
    void VulkanBase::createUniformBuffer(int count,MappedBuffer* mappedBuffer,size_t size)
    {
        VkDeviceSize bufferSize = size * count;

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //�ėp�I�ȃ��j�t�H�[���o�b�t�@�̍쐬�p
    void VulkanBase::createUniformBuffer(MappedBuffer* mappedBuffer, size_t size)
    {
        VkDeviceSize bufferSize = size;

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

    //UI�p
    void VulkanBase::createUniformBuffer(std::shared_ptr<UI> ui)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO2D);

        MappedBuffer* mappedBuffer = &ui->getMappedBuffer();

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer->uniformBuffer, mappedBuffer->uniformBufferMemory);

        vkMapMemory(device, mappedBuffer->uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer->uniformBufferMapped);
    }

    //�R���C�_�[�p��MVP�s��̃o�b�t�@�[���쐬
    void VulkanBase::createUniformBuffer(std::shared_ptr<Colider> colider)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO);

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
        poolInfo.maxSets = static_cast<uint32_t>(MAX_VKDESCRIPTORSET);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    //descriptorSet�̃o�b�t�@�[�̗p��
    //gltf���f���p
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

        if (descriptorSetCount > MAX_VKDESCRIPTORSET)
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
            TextureData* texture = gltfModel->imageDatas[material->baseColorTextureIndex]->getTexture();

            imageInfo[0].imageView = texture->view;
            imageInfo[0].sampler = texture->sampler;
        }
        if (material->metallicRoughnessTextureIndex != -1)
        {
            TextureData* texture = gltfModel->imageDatas[material->metallicRoughnessTextureIndex]->getTexture();

            imageInfo[1].imageView = texture->view;
            imageInfo[1].sampler = texture->sampler;
        }
        if (material->normalTextureIndex != -1)
        {
            TextureData* texture = gltfModel->imageDatas[material->normalTextureIndex]->getTexture();

            imageInfo[2].imageView = texture->view;
            imageInfo[2].sampler = texture->sampler;
        }
        if (material->occlusionTextureIndex != -1)
        {
            TextureData* texture = gltfModel->imageDatas[material->occlusionTextureIndex]->getTexture();

            imageInfo[3].imageView = texture->view;
            imageInfo[3].sampler = texture->sampler;
        }
        if (material->emissiveTextureIndex != -1)
        {
            TextureData* texture = gltfModel->imageDatas[material->emissiveTextureIndex]->getTexture();

            imageInfo[4].imageView = texture->view;
            imageInfo[4].sampler = texture->sampler;
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

    //���[�h��ʂ̕`��̏I��
    void VulkanBase::stopLoading()
    {
        //���ׂẴt�F���X������s�����̐M�����󂯎��
        vkWaitForFences(device, static_cast<uint32_t>(multiThreadFences.size()), multiThreadFences.data()
            , VK_TRUE, UINT64_MAX);
        //���ׂẴt�F���X�����Z�b�g
        vkResetFences(device, static_cast<uint32_t>(multiThreadFences.size()), multiThreadFences.data());

        //�L���[��̏��������ׂďI���܂ő҂�
        vkQueueWaitIdle(multiThreadGraphicQueue);
        vkQueueWaitIdle(multiThreadPresentQueue);
    }

    //���ׂẴ����_�����O������O�Ɏ��s�����
    void VulkanBase::renderBegin()
    {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    //���ׂẴ����_�����O���I������Ƃ����s�����
    void VulkanBase::renderEnd()
    {
        if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
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

        std::vector<VkSwapchainKHR> swapChains = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains.data();

        presentInfo.pImageIndices = &availableSwapChaneImageNumber;

        VkResult result;
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

    //gltf���f���̕`��
    void VulkanBase::drawMesh(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer
        , std::shared_ptr<Cubemap> cubemap, ShadowMapData& shadowMapData,PointLightBuffer& pointLightBuffer,DirectionalLightBuffer& dirLightBuffer)
    {
        Storage* storage = Storage::GetInstance();

        FragmentParam param = model->getFragmentParam();

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

        for (auto mesh : node->meshArray)
        {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffer, model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                std::shared_ptr<Material> material = model->getGltfModel()->materials[mesh->primitives[i].materialIndex];

                std::vector<VkDescriptorSet> descriptorSets =
                {
                    model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet,//MVP�s��ƃA�j���[�V�����s��
                    material->descriptorSet,//�}�e���A��
                    pointLightBuffer.descriptorSet,//�|�C���g���C�g
                    dirLightBuffer.descriptorSet,//���s����
                    shadowMapData.descriptorSets[0],//�V���h�E�}�b�v
                    cubemap->getDiffuse().descriptorSet,//IBL��diffuse
                    cubemap->getSpecularReflection().descriptorSet,//IBL��specular�̋��ʔ���
                    cubemap->getSpecularBRDF().descriptorSet//IBL��specular��BRDF
                };

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    node->descriptorInfo.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdPushConstants(commandBuffer, node->descriptorInfo.pLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(FragmentParam), &param);

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            drawMesh(node->children[i], model, commandBuffer, cubemap
                , shadowMapData,pointLightBuffer,dirLightBuffer);
        }
    }

    //�V���h�E�}�b�v�̍쐬
    void VulkanBase::calcDepth(GltfNode* node, std::shared_ptr<Model> model, VkCommandBuffer& commandBuffer,OffScreenPass& pass)
    {
        Storage* storage = Storage::GetInstance();

        for(auto mesh : node->meshArray)
        {
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

                vkCmdDrawIndexed(commandBuffer, mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            calcDepth(node->children[i], model, commandBuffer,pass);
        }
    }

    //�V���h�E�}�b�v�̃����_�����O�J�n
    void VulkanBase::shadowMapBegin(ShadowMapData& shadowMapData)
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

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //1.75f, 0.0f, 5.75f
        vkCmdSetDepthBias(commandBuffers[currentFrame], 1.25f, 0.0f, 1.75f);

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapData.passData.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)shadowMapData.passData.width;
        viewport.height = (float)shadowMapData.passData.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { (uint32_t)shadowMapData.passData.width,(uint32_t)shadowMapData.passData.height };
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
    }

    //�V���h�E�}�b�v�̃����_�����O
    void VulkanBase::renderShadowMap(std::shared_ptr<Model> model, ShadowMapData& shadowMapData)
    {
        GltfNode* node = model->getGltfModel()->getRootNode();

        for (auto mesh : node->meshArray)
        {
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            std::vector<VkDescriptorSet> descriptorSets(2);
            descriptorSets[0] = shadowMapData.passData.descriptorSets[0];

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                descriptorSets[1] = model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet;

                vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                    shadowMapData.passData.pipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdDrawIndexed(commandBuffers[currentFrame], mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            renderShadowMap(node->children[i], model, shadowMapData);
        }
    }

    //�V���h�E�}�b�v�̃����_�����O
    void VulkanBase::renderShadowMap(GltfNode* node,std::shared_ptr<Model> model, ShadowMapData& shadowMapData)
    {
        for (auto mesh : node->meshArray)
        {
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &model->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], model->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            std::vector<VkDescriptorSet> descriptorSets(2);
            descriptorSets[0] = shadowMapData.passData.descriptorSets[0];

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                descriptorSets[1] = model->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet;

                vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                    shadowMapData.passData.pipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdDrawIndexed(commandBuffers[currentFrame], mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            renderShadowMap(node->children[i], model, shadowMapData);
        }
    }

    //�V���h�E�}�b�v�̃����_�����O�I��
    void VulkanBase::shadowMapEnd()
    {
        vkCmdEndRenderPass(commandBuffers[currentFrame]);
    }

    //3D���f����UI�̃����_�����O���J�n
    void VulkanBase::sceneRenderBegin()
    {

        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX
            , imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &availableSwapChaneImageNumber);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[availableSwapChaneImageNumber];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2>clearValues{};
        clearValues[0].color = { {0.0f,0.0f,0.0f,1.0f} };
        clearValues[1].depthStencil = { 1.0f,0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    //3D���f���̃����_�����O
    void VulkanBase::renderModel(std::shared_ptr<Model> model, std::shared_ptr<Cubemap> cubemap
        ,ShadowMapData& shadowMapData, PointLightBuffer& pointLightBuffer, DirectionalLightBuffer& dirLightBuffer)
    {
        drawMesh(model->getRootNode(), model, commandBuffers[currentFrame], cubemap
            , shadowMapData, pointLightBuffer, dirLightBuffer);

        if (model->hasColider() && coliderDraw)
        {
            std::shared_ptr<Colider> colider = model->getColider();

            vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, modelDescriptor.coliderPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)swapChainExtent.width;
            viewport.height = (float)swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &colider->getPointBufferData()->vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], colider->getPointBufferData()->indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                modelDescriptor.coliderPipelineLayout, 0, 1, &colider->getDescSetData().descriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], colider->getDrawColiderIndicesSize(), 1, 0, 0, 0);
        }
    }

    //UI�̃����_�����O
    void VulkanBase::renderUI(std::shared_ptr<UI> ui)
    {
        if (!ui->getVisible())
        {
            return;
        }

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, uiRender.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        BufferObject& pointBuffer = ui->getPointBuffer()[currentFrame];

        vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &pointBuffer.vertBuffer, offsets);

        vkCmdBindIndexBuffer(commandBuffers[currentFrame], pointBuffer.indeBuffer, 0, VK_INDEX_TYPE_UINT32);

        std::array<VkDescriptorSet, 2> descriptorSets;
        descriptorSets = { ui->getTransformDescriptorSet(),ui->getImageDescriptorSet() };

        vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
            uiRender.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

        vkCmdDrawIndexed(commandBuffers[currentFrame], ui->getIndicesSize(), 1, 0, 0, 0);
    }

    //�e�L�X�gUI�̃����_�����O
    void VulkanBase::renderText(std::shared_ptr<Text> text)
    {
        if (!text->getVisible())
        {
            return;
        }

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, uiRender.fontPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        BufferObject& pointBuffer = text->getPointBuffer()[currentFrame];

        vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &pointBuffer.vertBuffer, offsets);

        vkCmdBindIndexBuffer(commandBuffers[currentFrame], pointBuffer.indeBuffer, 0, VK_INDEX_TYPE_UINT32);

        std::array<VkDescriptorSet, 2> descriptorSets =
        {
            text->getTransformDescriptorSet(),
            text->getImageDescriptorSet()
        };

        //VkDescriptorSet�Ƀt�H���g�̃A�g���X�e�N�X�`�����܂߂�
        vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
            uiRender.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

        vkCmdDrawIndexed(commandBuffers[currentFrame], text->getIndicesSize(), 1, 0, 0, 0);
    }

    //�L���[�u�}�b�v�̃����_�����O
    void VulkanBase::renderCubemap(std::shared_ptr<Cubemap> cubemap)
    {
        GltfNode* node = cubemap->getGltfModel()->getRootNode();

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, node->descriptorInfo.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        for (auto mesh : node->meshArray)
        {
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &cubemap->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], cubemap->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                std::vector<VkDescriptorSet> descriptorSets =
                {
                    cubemap->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet,
                    cubemap->getBackGroundColor().descriptorSet
                };

                vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                    node->descriptorInfo.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdDrawIndexed(commandBuffers[currentFrame], mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            renderCubemap(node->children[i], cubemap);
        }
    }


    //�L���[�u�}�b�v�̃����_�����O
    void VulkanBase::renderCubemap(GltfNode* node,std::shared_ptr<Cubemap> cubemap)
    {
        VkDeviceSize offsets[] = { 0 };

        for (auto mesh : node->meshArray)
        {
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &cubemap->getPointBufferData()[mesh->meshIndex].vertBuffer, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], cubemap->getPointBufferData()[mesh->meshIndex].indeBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (int i = 0; i < mesh->primitives.size(); i++)
            {
                std::vector<VkDescriptorSet> descriptorSets =
                {
                    cubemap->descSetDatas[mesh->primitives[i].primitiveIndex].descriptorSet,
                    cubemap->getBackGroundColor().descriptorSet
                };

                vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                    node->descriptorInfo.pLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

                vkCmdDrawIndexed(commandBuffers[currentFrame], mesh->primitives[i].indexCount, 1, mesh->primitives[i].firstIndex, 0, 0);
            }
        }

        for (int i = 0; i < node->children.size(); i++)
        {
            renderCubemap(node->children[i], cubemap);
        }
    }

    //3D���f����UI�̃����_�����O���I��
    void VulkanBase::sceneRenderEnd()
    {
        vkCmdEndRenderPass(commandBuffers[currentFrame]);
    }

    void VulkanBase::createSyncObjects() {//�`��̓��@�p�̕ϐ���p��
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

        VkPhysicalDeviceFeatures2 supportedFeatures{};
        supportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        vkGetPhysicalDeviceFeatures2(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported
            && swapChainAdequate && (supportedFeatures.features.samplerAnisotropy == VK_TRUE)
            && (supportedFeatures.features.shaderInt64 == VK_TRUE);
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
            extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
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

    //gltf���f���p��VkDesciptorSet�̍쐬(���C�L���X�g���g�p)
    void VulkanBase::createRaycastDescriptorSet(GltfNode* node, std::shared_ptr<GltfModel> model)
    {
        for (auto& mesh : node->meshArray)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &modelDescriptor.raycastLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &model->getRaycastDescriptorSet()[mesh->meshIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            {
                VkDescriptorBufferInfo info{};
                info.buffer = model->getPointBuffer()[mesh->meshIndex].vertBuffer;
                info.offset = 0;
                info.range = sizeof(Vertex) * static_cast<uint32_t>(mesh->vertices.size());

                VkDescriptorBufferInfo storageInfo{};
                storageInfo.buffer = model->getPointBuffer()[mesh->meshIndex].indeBuffer;
                storageInfo.offset = 0;
                storageInfo.range = sizeof(uint32_t) * static_cast<uint32_t>(mesh->indices.size());

                std::vector<VkWriteDescriptorSet> descriptorWrites(2);

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = model->getRaycastDescriptorSet()[mesh->meshIndex];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &info;

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = model->getRaycastDescriptorSet()[mesh->meshIndex];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pBufferInfo = &storageInfo;

                vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }

        for (auto& child : node->children)
        {
            createRaycastDescriptorSet(child, model);
        }
    }

    //gltf���f���̒��_�o�b�t�@�[�Ȃǂ̍쐬�A�t������R���C�_�[�̒��_�̃o�b�t�@�[���p��
    void VulkanBase::createMeshesData(std::shared_ptr<GltfModel> gltfModel)
    {
        createVertexBuffer(gltfModel->getRootNode(), gltfModel);
        createIndexBuffer(gltfModel->getRootNode(), gltfModel);

        createRaycastDescriptorSet(gltfModel->getRootNode(), gltfModel);
    }

    void VulkanBase::createMeshesData(std::shared_ptr<Colider> colider)
    {
        createVertexBuffer(colider);
        createIndexBuffer(colider);
    }

    //gltf���f���̊e�m�[�h�Ƀp�C�v���C���Ƃ��̃��C�A�E�g��ݒ肷��
    void VulkanBase::createDescriptorInfo(VkPipelineLayout& pLayout, VkPipeline& pipeline, GltfNode* node, std::shared_ptr<Model> model)
    {
        node->descriptorInfo.pLayout = pLayout;
        node->descriptorInfo.pipeline = pipeline;

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

    void VulkanBase::createDescriptorSetLayout()//�ʏ�̃����_�����O�ŕK�v��DescriptorSet�̃��C�A�E�g���쐬
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
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutBinding uboLayoutBindingAnimation{};
            uboLayoutBindingAnimation.binding = 1;
            uboLayoutBindingAnimation.descriptorCount = 1;
            uboLayoutBindingAnimation.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBindingAnimation.pImmutableSamplers = nullptr;
            uboLayoutBindingAnimation.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

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
            //�V���h�E�}�b�v�p�̃��C�A�E�g
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

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &shadowmapLayout) != VK_SUCCESS) {
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

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &iblLayout) != VK_SUCCESS)//IBL��diffuse
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
        
        {
            //���C�L���X�g�p�̃��C�A�E�g
            std::array<VkDescriptorSetLayoutBinding, 2> layoutBinding{};
            layoutBinding[0].binding = 0;
            layoutBinding[0].descriptorCount = 1;
            layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBinding[0].pImmutableSamplers = nullptr;
            layoutBinding[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            layoutBinding[1].binding = 1;
            layoutBinding[1].descriptorCount = 1;
            layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBinding[1].pImmutableSamplers = nullptr;
            layoutBinding[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.bindingCount = static_cast<uint32_t>(layoutBinding.size());
            info.pBindings = layoutBinding.data();

            if (vkCreateDescriptorSetLayout(device, &info, nullptr, &modelDescriptor.raycastLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to creat descriptor set layout");
            }
        }

        {
            //�t�H���g�p�̃��C�A�E�g
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = 0;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.bindingCount = 1;
            info.pBindings = &layoutBinding;

            if (vkCreateDescriptorSetLayout(device, &info, nullptr, &fontLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor set layout");
            }
        }

        {
            //�e�L�X�g�p�̃��C�A�E�g
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = 0;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.bindingCount = 1;
            info.pBindings = &layoutBinding;

            if (vkCreateDescriptorSetLayout(device, &info, nullptr, &textLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor set layout");
            }
        }
    }

    void VulkanBase::createPipelines()
    {
        /*�O���t�B�b�N�X�p�C�v���C�������*/
        createGraphicsPipeline("shaders/vert.spv", "shaders/frag.spv",
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, modelDescriptor.layout, modelDescriptor.texturePipelineLayout, modelDescriptor.texturePipeline);//���ʂ�3D���f���\���p

        createGraphicsPipeline("shaders/line.vert.spv", "shaders/line.frag.spv",
            VK_PRIMITIVE_TOPOLOGY_LINE_LIST, modelDescriptor.layout, modelDescriptor.coliderPipelineLayout, modelDescriptor.coliderPipeline);//�R���C�_�[�\���p
    }

    //���C�L���X�g�̃Z�b�g�A�b�v
    void VulkanBase::setupRaycast()
    {
        //���C�L���X�g�̃Z�b�g�A�b�v
        raycast.setup(device, commandPool, descriptorPool, modelDescriptor);
    }

    void VulkanBase::raycasting(VkCommandBuffer& commandBuffer, Ray& ray
        , GltfNode* node, std::shared_ptr<Model> model)
    {
        RaycastPushConstant pushConstant;
        pushConstant.pointer = uint64_t(node);

        for (auto& mesh : node->meshArray)
        {
            pushConstant.indexCount = static_cast<uint32_t>(mesh->indices.size());

            std::array<VkDescriptorSet, 3> descriptorSets =
            {
                raycast.descriptorSet,
                model->descSetDatas[mesh->meshIndex].descriptorSet,
                model->getGltfModel()->getRaycastDescriptorSet()[mesh->meshIndex]
            };

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, raycast.pipelineLayout,
                0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, 0);

            vkCmdPushConstants(commandBuffer, raycast.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RaycastPushConstant), &pushConstant);

            vkCmdDispatch(commandBuffer, static_cast<uint32_t>(mesh->indices.size()) * 2, 1, 1);
        }

        for (auto& child : node->children)
        {
            raycasting(commandBuffer, ray, child, model);
        }
    }

    //���C�L���X�g�̊J�n
    void VulkanBase::startRaycast(Ray& ray, std::shared_ptr<Model> model, float& distance,glm::vec3& faceNormal,GltfNode** node)
    {
        //��Ƀ��C�L���X�g���ɋA���Ă���f�[�^���󂯎��o�b�t�@���쐬
        createBuffer(sizeof(RaycastReturn), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            , raycast.storage.uniformBuffer, raycast.storage.uniformBufferMemory);

        //�X�e�[�W���O�o�b�t�@���쐬
        createBuffer(sizeof(RaycastReturn), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            , raycast.stagingBuffer.uniformBuffer, raycast.stagingBuffer.uniformBufferMemory);

        {
            //���C�L���X�g�J�n���̃o�b�t�@�̍X�V
            VkCommandBuffer copyCommand = beginSingleTimeCommands();
            raycast.startRaycast(ray, device, copyCommand);
            endSingleTimeCommands(copyCommand);
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(raycast.commandBuffer, &beginInfo);

        vkCmdBindPipeline(raycast.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, raycast.pipeline);

        raycasting(raycast.commandBuffer, ray, model->getRootNode(), model);

        vkEndCommandBuffer(raycast.commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &raycast.commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        auto submit = vkQueueSubmit(graphicsQueue, 1, &submitInfo, raycast.fence);
        if (submit != VK_SUCCESS) {
            throw std::runtime_error("failed to submit compute command buffer!");
        }

        raycast.waitFence(device);

        RaycastReturn obj{};
        obj.initilize();

        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
            raycast.getStorageBufferData(device, commandBuffer, obj);
            endSingleTimeCommands(commandBuffer);

            //�J�ڗp�o�b�t�@����l�����o��
            vkMapMemory(device, raycast.stagingBuffer.uniformBufferMemory, 0, sizeof(RaycastReturn)
                , 0, &raycast.stagingBuffer.uniformBufferMapped);
            memcpy(&obj, raycast.stagingBuffer.uniformBufferMapped, sizeof(RaycastReturn));
            vkUnmapMemory(device, raycast.stagingBuffer.uniformBufferMemory);

            raycast.storage.destroy(device);
            raycast.stagingBuffer.destroy(device);
        }

        //���C�L���X�g�̌��ʂ����o��
        //�Ȃ��A�ł��߂��q�b�g�̌��ʂ����o��

        distance = FLT_MAX;
        *node = nullptr;

        for (int i = 0; i < obj.pointer.size(); i++)
        {
            if (obj.pointer[i] != (uint64_t)0)
            {
                if (distance > obj.distance[i])
                {
                    distance = obj.distance[i];
                    faceNormal = obj.faceNormal[i];
                    (*node) = reinterpret_cast<GltfNode*>(obj.pointer[i]);
                }
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

    //�V���h�E�}�b�v�쐬�p��descriptorSet�֌W�̃f�[�^�̍쐬
    void VulkanBase::createDescriptorData_ShadowMap(std::vector<VkDescriptorSet>& descriptorSets,OffScreenPass& pass,VkDescriptorSetLayout& layout)
    {
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

    //�|�C���g���C�g��gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void VulkanBase::createPointLightBuffer(PointLightBuffer& buffer)
    {
        createUniformBuffer(1, &buffer.mappedBuffer, sizeof(PointLightUBO));

        createDescriptorData(buffer.mappedBuffer, modelDescriptor.lightLayout,
            buffer.descriptorSet, sizeof(PointLightUBO), VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    //���s������gpu��̃o�b�t�@�[�Ȃǂ��쐬
    void VulkanBase::createDirectionalLightBuffer(DirectionalLightBuffer& buffer)
    {
        createUniformBuffer(1, &buffer.mappedBuffer, sizeof(DirectionalLightUBO));

        createDescriptorData(buffer.mappedBuffer, modelDescriptor.lightLayout,
            buffer.descriptorSet, sizeof(DirectionalLightUBO), VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    //gltf���f���̎��}�e���A���p�̃f�[�^�̍쐬
    void VulkanBase::setGltfModelData(std::shared_ptr<GltfModel> gltfModel)
    {
        /*���_�A�C���f�b�N�X�o�b�t�@�[����������*/
        createMeshesData(gltfModel);

        /*�e�N�X�`���֘A�̐ݒ����������*/
        createTextureImage(gltfModel, VK_FORMAT_R8G8B8A8_UNORM);
        createTextureImageView(gltfModel, VK_FORMAT_R8G8B8A8_UNORM);
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
    void VulkanBase::prepareShadowMapping(int lightCount,ShadowMapData& shadowMap)
    {
        shadowMap.setFrameCount(lightCount);
        shadowMap.descriptorSets.resize(1);
        shadowMap.shadowMapScale = 4;

        shadowMap.proj = glm::ortho(shadowMapLeft, shadowMapRight, shadowMapBottom, shadowMapTop, shadowMapNear, shadowMapFar);

        shadowMap.passData.width = swapChainExtent.width * shadowMap.shadowMapScale;
        shadowMap.passData.height = swapChainExtent.height * shadowMap.shadowMapScale;

        for (auto& attachment : shadowMap.passData.imageAttachment)
        {
            createImage(shadowMap.passData.width, shadowMap.passData.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D16_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.memory);
            attachment.view = createImageView(attachment.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT, 1,1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, shadowMap.passData.sampler);

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

        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &shadowMap.passData.renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create renderpass");
        }

        // Create frame buffer
        for (int i = 0; i < shadowMap.passData.frameBuffer.size(); i++)
        {
            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = shadowMap.passData.renderPass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &shadowMap.passData.imageAttachment[i].view;
            fbufCreateInfo.width = shadowMap.passData.width;
            fbufCreateInfo.height = shadowMap.passData.height;
            fbufCreateInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &shadowMap.passData.frameBuffer[i]))
            {
                throw std::runtime_error("failed create frame buffer");
            }
        }

        for (int i = 0; i < lightCount; i++)
        {
            createUniformBuffer(1, &shadowMap.mappedBuffers[i], sizeof(ShadowMapUBO));
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

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &shadowMap.passData.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        for (int i = 0; i < lightCount; i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &shadowMap.passData.layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &shadowMap.passData.descriptorSets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = shadowMap.mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(ShadowMapUBO);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = shadowMap.passData.descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(1), &descriptorWrite, 0, nullptr);
        }

        createShadowMapPipeline("shaders/shadowMapping.vert.spv"
            , shadowMap.passData.layout, shadowMap.passData.pipelineLayout, shadowMap.passData.pipeline,shadowMap.passData.renderPass);

        createDescriptorData_ShadowMap(shadowMap.descriptorSets,shadowMap.passData,shadowmapLayout);//�f�v�X�o�b�t�@���e�N�X�`���Ƃ��ė��p���邽�߂�descriptorSet
    }

    //UI�`��p�̃p�C�v���C���Ȃǂ�p��
    void VulkanBase::prepareUIRendering()
    {
        uiRender.vertPath = "shaders/ui.vert.spv";
        uiRender.fragPath = "shaders/ui.frag.spv";
        uiRender.fragFontPath = "shaders/textUI.frag.spv";

        //VkDescriptorSetLayout�̍쐬
        {
            //�g�����X�t�H�[���p�̃��C�A�E�g
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = 0;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &layoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &uiRender.transformLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            //UI�摜�p�̃��C�A�E�g
            layoutBinding = VkDescriptorSetLayoutBinding{};
            layoutBinding.binding = 0;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            layoutInfo = VkDescriptorSetLayoutCreateInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &layoutBinding;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &uiRender.imageLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        //�p�C�v���C�����C�A�E�g�̍쐬
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

            std::array<VkPipelineShaderStageCreateInfo,2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

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
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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
            rasterizer.cullMode = VK_CULL_MODE_NONE;//�J�����O�𖳌�
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

            std::array<VkDescriptorSetLayout, 2> layouts =
            {
                uiRender.transformLayout,
                uiRender.imageLayout
            };

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
            pipelineLayoutInfo.pSetLayouts = layouts.data();

            //�摜�Ȃǂ̃����_�����O�p�̃p�C�v���C�����C�A�E�g�����
            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &uiRender.pLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages.data();
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

            //�摜�Ȃǂ̃����_�����O�p�̃p�C�v���C�������
            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &uiRender.pipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            vkDestroyShaderModule(device, fragShaderModule, nullptr);

            //�t�H���g�����_�����O�p�̃p�C�v���C�����C�A�E�g�����
            fragShaderCode = readFile(uiRender.fragFontPath);
            fragShaderModule = createShaderModule(fragShaderCode);

            fragShaderStageInfo.module = fragShaderModule;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &uiRender.fontPLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            //�t�H���g�Ȃǂ̃����_�����O�p�̃p�C�v���C�������
            pipelineInfo.layout = uiRender.fontPLayout;

            shaderStages = { vertShaderStageInfo,fragShaderStageInfo };
            
            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &uiRender.fontPipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            //�p�C�v���C���̍쐬

            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
        }

        //���[�hUI�̕\���p�̃R�}���h�o�b�t�@
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = multiThreadCommandPool;
            allocInfo.commandBufferCount = static_cast<uint32_t>(uiRender.loadCommandBuffers.size());

            vkAllocateCommandBuffers(device, &allocInfo, uiRender.loadCommandBuffers.data());
        }
    }

    //Model�N���X�̎��o�b�t�@�[�̍쐬
    void VulkanBase::setModelData(std::shared_ptr<Model> model)
    {
        if (model->hasColider())
        {
            createMeshesData(model->getColider());
        }

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
    void VulkanBase::prepareCubemapTextures(std::shared_ptr<Cubemap> cubemap)
    {
        BackGroundColor& backGroundColor = cubemap->getBackGroundColor();
        backGroundColor.srcHdriTexture = cubemap->getHDRIMap()->getTexture();

        backGroundColor.setFrameCount(CUBEMAP_FACE_COUNT);

        //�L���[�u�}�b�s���O�p�Ƀ����_�����O�C���[�W�𐳕��`�ɂ��Ă���
        std::shared_ptr<ImageData> cubemapImage = cubemap->getHDRIMap();
        int renderSize = std::min(cubemapImage->getWidth(), cubemapImage->getHeight());

        backGroundColor.passData.width = renderSize;
        backGroundColor.passData.height = renderSize;

        //�����_�����O�̌��ʂ̏o�͗p�̃o�b�t�@�̍쐬
        for (auto& attachment : backGroundColor.passData.imageAttachment)
        {
            createImage(renderSize,renderSize, 1, VK_SAMPLE_COUNT_1_BIT, backGroundColor.format,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.memory);
            attachment.view = createImageView(attachment.image, VK_IMAGE_VIEW_TYPE_2D, backGroundColor.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        }
        createImageSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, backGroundColor.passData.sampler);

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = backGroundColor.format;
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

        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &backGroundColor.passData.renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create renderpass");
        }

        // Create frame buffer
        for (int i = 0; i < backGroundColor.passData.frameBuffer.size(); i++)
        {
            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = backGroundColor.passData.renderPass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &backGroundColor.passData.imageAttachment[i].view;
            fbufCreateInfo.width = backGroundColor.passData.width;
            fbufCreateInfo.height = backGroundColor.passData.height;
            fbufCreateInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &backGroundColor.passData.frameBuffer[i]))
            {
                throw std::runtime_error("failed create frame buffer");
            }
        }

        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            createUniformBuffer(1, &backGroundColor.mappedBuffers[i], sizeof(MatricesUBO));
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

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &backGroundColor.passData.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &backGroundColor.passData.layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &backGroundColor.passData.descriptorSets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = backGroundColor.mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = backGroundColor.srcHdriTexture->view;
            info.sampler = backGroundColor.srcHdriTexture->sampler;

            std::vector<VkWriteDescriptorSet> descriptorWrites(2);
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = backGroundColor.passData.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = backGroundColor.passData.descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &info;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        createCalcCubeMapPipeline("shaders/calcCubemap.vert.spv","shaders/calcCubemap.frag.spv"
            , backGroundColor.passData.layout, backGroundColor.passData.pipelineLayout, backGroundColor.passData.pipeline, backGroundColor.passData.renderPass);
    }

    void VulkanBase::prepareIBL(std::string vertShaderPath, std::string fragShaderPath
        ,OffScreenPass& passData,VkFormat format,uint32_t mipmapLevel,std::vector<MappedBuffer>& mappedBuffers, std::shared_ptr<Cubemap> cubemap)
    {
        //�����_�����O�̌��ʂ̏o�͗p�̃o�b�t�@�̍쐬
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

            //�L���[�u�}�b�s���O�Ŏg���s��̃o�b�t�@�𗬗p����
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            //6�̃��C���[�����摜���Q�Ƃ���悤�ɂ���
            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = cubemap->getBackGroundColor().multiTexture->view;
            info.sampler = cubemap->getBackGroundColor().multiTexture->sampler;

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
    void VulkanBase::prepareIBL(IBLSpecularReflection& iblSpecular, std::shared_ptr<Cubemap> cubemap)
    {
        //�����_�����O�̌��ʂ̏o�͗p�̃o�b�t�@�̍쐬
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

            //�L���[�u�}�b�s���O�Ŏg���s��̃o�b�t�@�𗬗p����
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = cubemap->getBackGroundColor().mappedBuffers[i].uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatricesUBO);

            //6�̃��C���[�����摜���Q�Ƃ���悤�ɂ���
            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = cubemap->getBackGroundColor().multiTexture->view;
            info.sampler = cubemap->getBackGroundColor().multiTexture->sampler;

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

    //6���̃e�N�X�`�����쐬���āA�L���[�u�}�b�v���쐬
    void VulkanBase::createSamplerCube2D(OffScreenPass& passData,std::vector<MappedBuffer>& mappedBuffers, std::shared_ptr<Cubemap> cubemap)
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
            //�J�����𗧕��̂̓�������A6�̖ʂ��f���o���悤�ɁA�r���[�s����X�V���Ă���

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

            drawSamplerCube(storage->getgltfModel(cubemapPath)->getRootNode(), cubemap
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

    //specular�p��SamplerCube�����֐�
    void VulkanBase::createSamplerCube2D(IBLSpecularReflection& iblSpecular, std::vector<MappedBuffer>& mappedBuffers, std::shared_ptr<Cubemap> cubemap)
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

                drawSamplerCube(storage->getgltfModel(cubemapPath)->getRootNode(), cubemap
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

    //specular�p��SamplerCube�����֐�
    void VulkanBase::createLUT(IBLSpecularBRDF& iblSpecular, MappedBuffer& mappedBuffer,std::shared_ptr<Cubemap> cubemap)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        Storage* storage = Storage::GetInstance();

        //��̖ʂ̂݃����_�����O����
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

        drawSamplerCube(storage->getgltfModel(cubemapPath)->getRootNode(), cubemap
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

    //�L���[�u�}�b�v�̍쐬
    void VulkanBase::createCubemap(std::shared_ptr<Cubemap> cubemap)
    {
        BackGroundColor& backGroundColor = cubemap->getBackGroundColor();
        IBLDiffuse& diffuse = cubemap->getDiffuse();
        IBLSpecularReflection& reflection = cubemap->getSpecularReflection();
        IBLSpecularBRDF& brdf = cubemap->getSpecularBRDF();

        backGroundColor.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        diffuse.format = backGroundColor.format;
        reflection.format = backGroundColor.format;

        prepareCubemapTextures(cubemap);//�L���[�u�}�b�s���O�̗p��

        /*UnifomBuffer����������*/
        createUniformBuffers(cubemap);

        //6���̃e�N�X�`�����쐬����
        createSamplerCube2D(backGroundColor.passData, backGroundColor.mappedBuffers,cubemap);

        /*�X�J�C�{�b�N�X�p�̃e�N�X�`����p�ӂ���*/
        {
            {
                //�X�J�C�{�b�N�X�p�Ƀ��C���[�̐���6��
                std::shared_ptr<ImageData> image = cubemap->getHDRIMap();;
                int cubemapImageSize = std::min(image->getWidth(), image->getHeight());

                //�t���[���o�b�t�@����L���[�u�}�b�v�̃����_�����O���ʂ��擾��
                //�t���[���o�b�t�@�̉摜�̃��C�A�E�g�𑗐M�p�ɂ���
                {
                    for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
                    {
                        transitionImageLayout(backGroundColor.passData.imageAttachment[i].image,
                            backGroundColor.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);
                    }
                }

                backGroundColor.multiTexture->mipLevel = calcMipMapLevel(cubemapImageSize, cubemapImageSize);

                {//6�̃��C���[�����e�N�X�`���f�[�^���쐬
                    createMultiLayerTexture(backGroundColor.multiTexture, CUBEMAP_FACE_COUNT
                        , cubemapImageSize, cubemapImageSize, backGroundColor.multiTexture->mipLevel, backGroundColor.format);

                    //�摜�̂��ׂẴ��C���[���ړ�������
                    transitionImageLayout(backGroundColor.multiTexture->image, backGroundColor.format
                        , VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, backGroundColor.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);

                    std::vector<VkImage> srcImages(CUBEMAP_FACE_COUNT);
                    for (uint32_t i = 0;i < backGroundColor.passData.imageAttachment.size();i++)
                    {
                        srcImages[i] = backGroundColor.passData.imageAttachment[i].image;
                    }
                    copyImageToMultiLayerImage(srcImages.data(), static_cast<uint32_t>(srcImages.size())
                        , backGroundColor.passData.width, backGroundColor.passData.height, backGroundColor.multiTexture->image);

                    generateMipmaps(backGroundColor.multiTexture->image, backGroundColor.format
                        , cubemapImageSize, cubemapImageSize, backGroundColor.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);
                }
            }

            //�r���[���e�N�X�`���L���[�u�ɐݒ�
            backGroundColor.multiTexture->view = createImageView(backGroundColor.multiTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, backGroundColor.format, VK_IMAGE_ASPECT_COLOR_BIT
                , backGroundColor.multiTexture->mipLevel, CUBEMAP_FACE_COUNT);

            createTextureSampler(backGroundColor.multiTexture);
        }

        createCubemapPipeline("shaders/cubemap.vert.spv", "shaders/cubemap.frag.spv", iblLayout, backGroundColor.pipelineLayout, backGroundColor.pipeline, renderPass);

        /*��������p�C�v���C���́A�����O���[�v�̃��f���ł͎g���܂킹��*/
        /*�f�B�X�N���v�^�Z�b�g�́A�e�N�X�`���f�[�^���قȂ�ꍇ�͎g���܂킹�Ȃ�*/
        createDescriptorInfos(backGroundColor.pipelineLayout, backGroundColor.pipeline, cubemap);

        //SamplerCube�p��descriptorSet�̍쐬
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
            allocInfo.pSetLayouts = &iblLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &backGroundColor.descriptorSet) != VK_SUCCESS)
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
            imageInfo.imageView = backGroundColor.multiTexture->view;
            imageInfo.sampler = backGroundColor.multiTexture->sampler;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = backGroundColor.descriptorSet;
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
        createIBL(diffuse,reflection,brdf,cubemap);

        cubemap->getGltfModel()->setup = true;
    }

    //IBL���쐬����
    void VulkanBase::createIBL(IBLDiffuse& diffuse,IBLSpecularReflection& reflection,IBLSpecularBRDF& brdf,std::shared_ptr<Cubemap> cubemap)
    {
        //diffuse
        createIBLDiffuse(diffuse,cubemap);
        //specular
        createIBLSpecular(reflection,brdf,cubemap);
    }

    //IBL��diffuse�e�N�X�`�����쐬����
    void VulkanBase::createIBLDiffuse(IBLDiffuse& diffuse,std::shared_ptr<Cubemap> cubemap)
    {
        //���O�v�Z�p�̃V�F�[�_�̐ݒ�
        diffuse.vertShaderPath = "shaders/calcIBL.vert.spv";
        diffuse.fragShaderPath = "shaders/calcDiffuse.frag.spv";

        //�I�t�X�N���[�������_�����O�p�̃t���[�������쐬����
        diffuse.setFrameCount(CUBEMAP_FACE_COUNT);
        //�e�N�X�`���̃T�C�Y�ݒ�
        diffuse.setRenderSize(IBL_MAP_SIZE);

        //�~�b�v�}�b�v���x���̌v�Z
        diffuse.setMipmapLevel(1);

        //diffuse�p�e�N�X�`����6��̃I�t�X�N���[�������_�����O�̏������s��
        prepareIBL(diffuse.vertShaderPath, diffuse.fragShaderPath
            , diffuse.passData, diffuse.format,diffuse.mipmapLevel,diffuse.mappedBuffers,cubemap);

        //SamperCube���쐬����
        createSamplerCube2D(diffuse.passData, diffuse.mappedBuffers,cubemap);

        //6�̎��_�̉摜���܂Ƃ߂āASamplerCube���쐬����
        createCubeMapTextureFromImages(diffuse.size, diffuse.mipmapLevel, diffuse.multiLayerTexture, diffuse.passData.imageAttachment, diffuse.format);

        //�ʏ탌���_�����O���Ɏg��DescriptorSet�֘A�̃f�[�^���쐬
        createIBLDescriptor(diffuse.multiLayerTexture, iblLayout, diffuse.descriptorSet);
    }

    //IBL��specular�e�N�X�`�����쐬����
    void VulkanBase::createIBLSpecular(IBLSpecularReflection& reflection,IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap)
    {
        //specular�̋��ʔ��ˍ��̃}�b�v���쐬
        createIBLSpecularReflection(reflection,cubemap);
        //specular��BRDF�̃}�b�v���쐬
        createIBLSpecularBRDF(brdf,cubemap);
    }

    void VulkanBase::createIBLSpecularReflection(IBLSpecularReflection& reflection, std::shared_ptr<Cubemap> cubemap)
    {
        //���O�v�Z�p�̃V�F�[�_��ݒ�
        reflection.vertShaderPath = "shaders/calcIBL.vert.spv";
        reflection.fragShaderPath = "shaders/calcSpecularReflection.frag.spv";

        //�e�N�X�`���̃T�C�Y�̐ݒ�
        reflection.setRenderSize(IBL_MAP_SIZE);

        //�I�t�X�N���[�������_�����O�̃t���[������ݒ肷��
        reflection.setFrameCount(CUBEMAP_FACE_COUNT);

        //specular�p�e�N�X�`����6��̃I�t�X�N���[�������_�����O�̏������s��
        prepareIBL(reflection,cubemap);

        //6�̎��_����̗����̂̓����������_�����O���āA6�̎��_���ꂼ��̉摜���~�b�v�}�b�v�̐������쐬
        createSamplerCube2D(reflection, reflection.mappedBuffers,cubemap);

        //6�̎��_�̉摜���܂Ƃ߂āASamplerCube���쐬����
        createCubeMapTextureFromImages(reflection.size, reflection.multiLayerTexture
            , reflection.imageAttachment, reflection.mipmapLevelSize,reflection.format);

        //�ʏ탌���_�����O���Ɏg��DescriptorSet�֘A�̃f�[�^���쐬
        createIBLDescriptor(reflection.multiLayerTexture, iblLayout, reflection.descriptorSet);
    }

    //specular��BRDF���ɂ��Ẵ}�b�v���쐬
    void VulkanBase::createIBLSpecularBRDF(IBLSpecularBRDF& brdf, std::shared_ptr<Cubemap> cubemap)
    {
        //���O�v�Z�p�̃V�F�[�_��ݒ�
        brdf.vertShaderPath = "shaders\\calcBRDF.vert.spv";
        brdf.fragShaderPath = "shaders\\calcSpecularBRDF.frag.spv";

        //�e�N�X�`���̃T�C�Y�̐ݒ�
        brdf.setRenderSize(IBL_MAP_SIZE);

        //IBL��speuclar��BRDF�ł́A�����̂̈�ʂ̂݃����_�����O����΂���
        brdf.setFrameCount(1);

        //specular�p�e�N�X�`����6��̃I�t�X�N���[�������_�����O�̏������s��
        prepareIBL(brdf.vertShaderPath, brdf.fragShaderPath
            , brdf.passData, VK_FORMAT_R16G16B16A16_SFLOAT,1,brdf.mappedBuffers,cubemap);

        //��̎��_�݂̂��痧���̖̂ʂ������_�����O���āA2D��LUT���쐬����
        createLUT(brdf, brdf.mappedBuffers[0],cubemap);

        //�����_�����O�����摜���V�F�[�_�œǂݎ�邽�߂̃e�N�X�`���ɕϊ�����
        transitionImageLayout(brdf.passData.imageAttachment[0].image, VK_FORMAT_R16G16B16A16_SFLOAT
            , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

        //�����̂̈�̖ʂ݂̂̃����_�����O���ʂ�LUT�Ƃ��ė��p
        //�ʏ탌���_�����O���Ɏg��DescriptorSet�֘A�̃f�[�^���쐬
        createIBLDescriptor(brdf.passData, iblLayout, brdf.descriptorSet);
    }

    //6�̉摜����̉摜�ɂ܂Ƃ߂āASamplerCube�����
    void VulkanBase::createCubeMapTextureFromImages(uint32_t texSize,uint32_t srcTextureMipmapLevel, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment,VkFormat format)
    {
        //6�̃��C���[�����摜�����
        createMultiLayerTexture(multiLayerTexture, CUBEMAP_FACE_COUNT, texSize, texSize, multiLayerTexture->mipLevel,format);

        //�����_�����O����6���̉摜��]���p�̃��C�A�E�g�ɕύX����
        for (int i = 0; i < imageAttachment.size(); i++)
        {
            transitionImageLayout(imageAttachment[i].image,
                format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcTextureMipmapLevel, 1);
        }

        //SamplerCube�̉摜�̂��ׂẴ��C���[���ړ�������
        transitionImageLayout(multiLayerTexture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
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
        generateMipmaps(multiLayerTexture->image, format,
            texSize, texSize, multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        //�r���[���e�N�X�`���L���[�u�ɐݒ�
        multiLayerTexture->view = createImageView(multiLayerTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, format, VK_IMAGE_ASPECT_COLOR_BIT
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        createTextureSampler(multiLayerTexture);
    }

    //6�̉摜����̉摜�ɂ܂Ƃ߂āASamplerCube�����
    void VulkanBase::createCubeMapTextureFromImages(uint32_t texSize, TextureData* multiLayerTexture
        , std::vector<FrameBufferAttachment>& imageAttachment, std::vector<uint32_t>& mipmapLevelSize,VkFormat format)
    {
        //6�̃��C���[�����摜�����
        createMultiLayerTexture(multiLayerTexture, CUBEMAP_FACE_COUNT, texSize, texSize, multiLayerTexture->mipLevel,format);

        //�����_�����O����6���̉摜��]���p�̃��C�A�E�g�ɕύX����
        for (int i = 0; i < imageAttachment.size(); i++)
        {
            transitionImageLayout(imageAttachment[i].image,
                format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);
        }

        //SamplerCube�̉摜�̂��ׂẴ��C���[���ړ�������
        transitionImageLayout(multiLayerTexture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);


        //���܂ō쐬�����𑜓x�̈Ⴄ�摜���A����̎��_�̉摜�̃~�b�v�}�b�v�Ƃ��Ċi�[����
        //�������_�̃����_�����O�摜���W�߂āA������~�b�v�}�b�v�Ƃ��Ă܂Ƃ߂Ă���
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            for (int i = 0; i < CUBEMAP_FACE_COUNT; i++)
            {
                //���炩���߃����_�����O���Ă������e�ʂ�specular�̃e�N�X�`���̑f�ނ�
                //�����ŁA6�̃��C���[�������A�����̃~�b�v�}�b�v�����摜�f�[�^�ɂЂƂ܂Ƃ߂ɂ���

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

        //�~�b�v�}�b�v�̍쐬
        generateMipmaps(multiLayerTexture->image, format,
            texSize, texSize, multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        //�r���[���e�N�X�`���L���[�u�ɐݒ�
        multiLayerTexture->view = createImageView(multiLayerTexture->image, VK_IMAGE_VIEW_TYPE_CUBE, format, VK_IMAGE_ASPECT_COLOR_BIT
            , multiLayerTexture->mipLevel, CUBEMAP_FACE_COUNT);

        createTextureSampler(multiLayerTexture);
    }

    //IBL�p��DescriptorSet�̗p��
    void VulkanBase::createIBLDescriptor(TextureData* samplerCube, VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet)
    {
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

    //UI�̒��_�o�b�t�@�Ȃǂ�p�ӂ���
    void VulkanBase::setUI(std::shared_ptr<UI> ui)
    {
        for (int i = 0; i < ui->getPointBufferSize(); i++)
        {
            BufferObject& buffer = ui->getPointBuffer()[i];

            if (buffer.vertHandler)
            {
                addDefferedDestructBuffer(buffer);
            }
        }

        //���_�o�b�t�@�̍쐬
        createVertexBuffer(ui);
        //�C���f�b�N�X�o�b�t�@�̍쐬
        createIndexBuffer(ui);

        //VkDescriptorSet�̃o�b�t�@�[��p��
        allocateUIDescriptorSet(ui);

        //�o�b�t�@�[�����ѕt����
        createUIDescriptorSet(ui->getUITexture(), ui->getMappedBuffer()
            , ui->getTransformDescriptorSet(),ui->getImageDescriptorSet());
    }

    //�e�L�X�g�̒��_�o�b�t�@�Ȃǂ�p�ӂ���
    void VulkanBase::setText(std::shared_ptr<Text> text)
    {
        for (int i = 0;i < text->getPointBufferSize();i++)
        {
            BufferObject& buffer = text->getPointBuffer()[i];
            
            if (buffer.vertHandler)
            {
                addDefferedDestructBuffer(buffer);
            }
        }

        //���_�o�b�t�@�̍쐬
        createVertexBuffer(text);
        //�C���f�b�N�X�o�b�t�@�̍쐬
        createIndexBuffer(text);

        //VkDescriptorSet�̃o�b�t�@�[��p��
        allocateUIDescriptorSet(text);

        MappedBuffer mappedBuffer = text->getMappedBuffer();

        //���W�ϊ��s��p�̃o�b�t�@�̍쐬
        createUIDescriptorSet(mappedBuffer, text->getTransformDescriptorSet());
    }

    //�t�H���g�����_�����O�p��VkDescriptorSet��p��
    void VulkanBase::allocateUIDescriptorSet(std::shared_ptr<UI> ui)
    {
        //VkDescriptorSet�̃o�b�t�@���m��
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &uiRender.transformLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &ui->getTransformDescriptorSet()) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            allocInfo = VkDescriptorSetAllocateInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &uiRender.imageLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &ui->getImageDescriptorSet()) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;
        }
    }

    //�t�H���g�����_�����O�p��VkDescriptorSet��p��
    void VulkanBase::allocateUIDescriptorSet(std::shared_ptr<Text> text)
    {
        //VkDescriptorSet�̃o�b�t�@���m��
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &uiRender.transformLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &text->getTransformDescriptorSet()) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }
        }
    }

    //UI��uniform buffer�̍쐬
    void VulkanBase::uiCreateUniformBuffer(MappedBuffer& mappedBuffer)
    {
        VkDeviceSize bufferSize = sizeof(MatricesUBO2D);

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedBuffer.uniformBuffer, mappedBuffer.uniformBufferMemory);

        vkMapMemory(device, mappedBuffer.uniformBufferMemory, 0, bufferSize, 0, &mappedBuffer.uniformBufferMapped);
    }

    //�t�H���g�p��VkDescriptorSet���쐬����
    void VulkanBase::createFontDescriptorSet(std::shared_ptr<ImageData> atlasTexture,VkDescriptorSet& descriptorSet)
    {
        //VkDescriptorSet�̃o�b�t�@���m��
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &uiRender.imageLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            if (descriptorSetCount > MAX_VKDESCRIPTORSET)
            {
                throw std::runtime_error("allocateDescriptorSets: DescriptorSet overflow");
            }
            descriptorSetCount++;
        }

        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = atlasTexture->getTexture()->view;
            imageInfo.sampler = atlasTexture->getTexture()->sampler;

            VkWriteDescriptorSet descriptorWrite{};

            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    //UI�̃e�N�X�`���̕ύX�𔽉f
    void VulkanBase::createUIDescriptorSet(TextureData* textureData, MappedBuffer& mappedBuffer
        , VkDescriptorSet& transformDescriptorSet, VkDescriptorSet& imageDescriptorSet)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mappedBuffer.uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatricesUBO2D);

        VkWriteDescriptorSet descriptorWrite{};

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = transformDescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureData->view;
        imageInfo.sampler = textureData->sampler;

        descriptorWrite = VkWriteDescriptorSet{};

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = imageDescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    //UI�p�̃e�N�X�`���𒣂�t����|���S���̂��߂̃o�b�t�@��VkDescriptorSet�����
    void VulkanBase::createUIDescriptorSet(MappedBuffer& mappedBuffer, VkDescriptorSet& descriptorSet)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mappedBuffer.uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatricesUBO2D);

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

    //gpu�̃o�b�t�@��j�����X�g�ɒǉ�
    void VulkanBase::addDefferedDestructBuffer(BufferObject& pointBuffer)
    {
        uint32_t frameNum = (currentFrame == 0) ? 1 : 0;

        defferedDestruct.bufferList[frameNum].push_back(pointBuffer.vertBuffer);
        defferedDestruct.memoryList[frameNum].push_back(pointBuffer.vertHandler);

        defferedDestruct.bufferList[frameNum].push_back(pointBuffer.indeBuffer);
        defferedDestruct.memoryList[frameNum].push_back(pointBuffer.indeHandler);
    }

    void VulkanBase::addDefferedDestructBuffer(MappedBuffer& mappedBuffer)
    {
        uint32_t frameNum = (currentFrame == 0) ? 1 : 0;

        defferedDestruct.bufferList[frameNum].push_back(mappedBuffer.uniformBuffer);
        defferedDestruct.memoryList[frameNum].push_back(mappedBuffer.uniformBufferMemory);
    }

    void VulkanBase::addDefferedDestructBuffer(VkBuffer& buffer, VkDeviceMemory& memory)
    {
        uint32_t frameNum = (currentFrame == 0) ? 1 : 0;

        defferedDestruct.bufferList[frameNum].push_back(buffer);
        defferedDestruct.memoryList[frameNum].push_back(memory);
    }

    //gpu�̃o�b�t�@��j��
    void VulkanBase::cleanupDefferedBuffer()
    {
        uint32_t frameNum = (currentFrame == 0) ? 1 : 0;

        //�����_�����O�I����҂�
        //�t�F���X�̃��Z�b�g�͂��Ȃ�
        vkWaitForFences(device, 1, &inFlightFences[frameNum], true, UINT64_MAX);

        for (auto& buffer : defferedDestruct.bufferList[frameNum])
        {
            vkDestroyBuffer(device, buffer, nullptr);
        }

        for (auto& memory : defferedDestruct.memoryList[frameNum])
        {
            vkFreeMemory(device, memory, nullptr);
        }

        defferedDestruct.bufferList[frameNum].clear();
        defferedDestruct.memoryList[frameNum].clear();
    }

    //���X�g��̃o�b�t�@�����ׂĔj������
    void VulkanBase::allCleanupDefferedBuffer()
    {
        //�����_�����O�I����҂�
        //�t�F���X�̃��Z�b�g�͂��Ȃ�
        vkWaitForFences(device, static_cast<uint32_t>(inFlightFences.size()), inFlightFences.data(), true, UINT64_MAX);

        for (int i = 0; i < swapChainFramebuffers.size(); i++)
        {
            for (auto& buffer : defferedDestruct.bufferList[i])
            {
                vkDestroyBuffer(device, buffer, nullptr);
            }

            for (auto& memory : defferedDestruct.memoryList[i])
            {
                vkFreeMemory(device, memory, nullptr);
            }

            defferedDestruct.bufferList[i].clear();
            defferedDestruct.memoryList[i].clear();
        }
    }