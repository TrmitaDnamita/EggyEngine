#include "VulkanEngine.hpp"

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace EggyEngine {

    Engine::Engine() {

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        _window = glfwCreateWindow(WIDTH, HEIGHT, "9/11 was a inside job",
            nullptr, //No Fullscreen
            nullptr  //No Shared Resources 
        );

        if (_window == nullptr)
            Debug::errorWindow(L"Unable to create glfw window");

        startEngine();
    }

    Engine::~Engine() {

        destroyPipeline();
        destroySwapChain();
        destroyFrameBuffers();

        vkDestroyDevice(_vkDevice, nullptr);

        destroyInstance();
        destroyWindow();
    }

    void Engine::destroyWindow() {
        
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void Engine::destroyInstance(){
        
        if (enableValidationLayers)
            Debug::destroyDebugUtilsMessengerEXT(_vkInstance, nullptr);

        vkDestroyInstance(_vkInstance, nullptr);
    }

    void Engine::destroyFrameBuffers() {
        
        for (auto framebuffer : _swapChainFramebuffers)
            vkDestroyFramebuffer(_vkDevice, framebuffer, nullptr);
        

    }

    void Engine::destroySwapChain(){
        
        for (auto imageView : _swapChainImageViews)
            vkDestroyImageView(_vkDevice, imageView, nullptr);

        vkDestroySwapchainKHR(_vkDevice, _vkSwapChain, nullptr);

        vkDestroySurfaceKHR(_vkInstance, _vkSurface, nullptr);
    }

    void Engine::destroyPipeline(){
        
        vkDestroyShaderModule(_vkDevice, _fragShaderModule, nullptr);
        vkDestroyShaderModule(_vkDevice, _vertShaderModule, nullptr);

        vkDestroyPipeline(_vkDevice, _vkGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_vkDevice, _vkPipelineLayout, nullptr);
        vkDestroyRenderPass(_vkDevice, _vkRenderPass, nullptr);
    }

//Window Pass

    void Engine::run()
    {
        while (true) {
            if (glfwWindowShouldClose(_window))
                return;

            glfwPollEvents();
        }
    }

    void Engine::startEngine() {

        createInstance();

        createSwapChain();

        createPipeline();

        createFramebuffers();
    }

//End Pass

//Instance Pass

	void Engine::createInstance() {

        if (enableValidationLayers)
            if (!Debug::checkValidationLayerSupport())
                Debug::errorWindow(L"validation layers requested, but not available!");

        VkApplicationInfo _appInfo {

            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Hello Triangle",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "EggyEngine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0

        };

        // Vulkan Instance Information Struct

        auto extensions = Debug::getRequiredExtensions();

        VkInstanceCreateInfo _instanceInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &_appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()

        };

        if (!enableValidationLayers) {
            if (vkCreateInstance(&_instanceInfo, nullptr, &_vkInstance) != VK_SUCCESS)
                Debug::errorWindow(L"failed to create instance!");

            return;
        }

        _instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        _instanceInfo.ppEnabledLayerNames = validationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        Debug::populateDebugMessengerCreateInfo(debugCreateInfo);

        _instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

        if (vkCreateInstance(&_instanceInfo, nullptr, &_vkInstance) != VK_SUCCESS)
            Debug::errorWindow(L"failed to create instance!");

        if (Debug::CreateDebugUtilsMessengerEXT(_vkInstance, &debugCreateInfo, nullptr) != VK_SUCCESS)
            Debug::errorWindow(L"validation enabled but failed to create debug messenger!");
	    
    }

//End Pass

//SwapChain Pass

    int Engine::rateDeviceSuitability(VkPhysicalDevice device) {
        int score = 0;

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader) {
            return 0;
        }

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        return score;
    }

    bool Engine::checkDeviceExtensionSupport() {

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    void Engine::findQueueFamilies() {
        // Logic to find graphics queue family

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {

            vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _vkSurface, &indices.presentFamilySet);
            indices.graphicsFamilySet = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;

            if (indices.presentFamilySet)
                indices.presentFamily = i;

            if (indices.graphicsFamilySet)
                indices.graphicsFamily = i;

            if (indices.isComplete())
                break;

            i++;
        }
    }

    SwapChainSupportDetails Engine::querySwapChainSupport() {

        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _vkSurface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _vkSurface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _vkSurface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _vkSurface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _vkSurface, &presentModeCount, details.presentModes.data());
        }

        if (details.presentModes.empty() || details.formats.empty())
            Debug::errorWindow(L"GPU found but support for SwapChain not found");

        return details;
    }
    
    VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                )
                return availableFormat;

        return availableFormats[0];
    }

    VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(_physicalDevice, &deviceProperties);

        switch (deviceProperties.deviceType) {
            // This is a discrete GPU (e.g. Nvidia or AMD)
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return VK_PRESENT_MODE_MAILBOX_KHR; break;
            // This is an integrated GPU (e.g. Intel HD Graphics)
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return VK_PRESENT_MODE_FIFO_RELAXED_KHR; break;
            // Other device type
        default: return VK_PRESENT_MODE_FIFO_KHR; break;
        }
    }

    VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(_window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
    
    void Engine::createSurface() {

        if (glfwCreateWindowSurface(_vkInstance, _window, nullptr, &_vkSurface) != VK_SUCCESS)
            Debug::errorWindow(L"failed to create window surface!");

    }

    void Engine::pickPhysicalDevice() {

        uint32_t deviceCount = 0;

        vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
            Debug::errorWindow(L"failed to find GPUs with Vulkan support!");

        //If count is higher than zero, this time we extract all the devices with the vector devices.
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, devices.data());

        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices) {
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0)
            _physicalDevice = candidates.rbegin()->second;
        else
            Debug::errorWindow(L"failed to find a suitable GPU!");
        
        if (_physicalDevice == VK_NULL_HANDLE)
            Debug::errorWindow(L"failed to find a suitable GPU!");

        if (!checkDeviceExtensionSupport())
            Debug::errorWindow(L"failed to find a suitable GPU! - Device does not support all Extentions required");
    }

    void Engine::createLogicalDevice() {

        findQueueFamilies();

        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily,
            indices.presentFamily
        };

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());

        const float queuePriority = 1.0f;

        int index = 0;

        for (uint32_t queueFamily : uniqueQueueFamilies) {

            queueCreateInfos[index].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[index].queueCount = 1;
            queueCreateInfos[index].queueFamilyIndex = queueFamily;
            queueCreateInfos[index].pQueuePriorities = &queuePriority;

            index++;
        }
        
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
        };

        if (enableValidationLayers) {

            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }

        if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_vkDevice) != VK_SUCCESS)
            Debug::errorWindow(L"failed to create logical device!");

        vkGetDeviceQueue(_vkDevice, indices.presentFamily, 0, &_presentQueue);
    }

    void Engine::startSwapChain() {

        SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount--;

        uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

        VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = _vkSurface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = 2,
            .pQueueFamilyIndices = queueFamilyIndices,
            .preTransform = swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE // if resized we need to recreate the swapchain
        };

        if (indices.indicesMatch()) {

            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0;
            swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        }

        if (vkCreateSwapchainKHR(_vkDevice, &swapchainCreateInfo, nullptr, &_vkSwapChain) != VK_SUCCESS)
            Debug::errorWindow(L"failed to create swap chain!");

        vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &imageCount, nullptr);
        _swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &imageCount, _swapChainImages.data());

        _swapChainImageFormat = surfaceFormat.format;
        _swapChainExtent = extent;

    }
    
    void Engine::createImageViews() {

        _swapChainImageViews.resize(_swapChainImages.size());

        VkImageViewCreateInfo viewCreateInfo {};

        viewCreateInfo.pNext = nullptr;
        viewCreateInfo.flags = 0;
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = _swapChainImageFormat;

        viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        for (size_t i = 0; i < _swapChainImages.size(); i++) {

            viewCreateInfo.image = _swapChainImages[i];
            
            if (vkCreateImageView(_vkDevice, &viewCreateInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
                Debug::errorWindow(L"failed to create image views!");
        }
    }
    
    void Engine::createSwapChain() {
        
        createSurface();

        pickPhysicalDevice();

        createLogicalDevice();

        startSwapChain();

        createImageViews();
    }
//End Pass

//Shader Related Pass
    
    VkPipelineShaderStageCreateInfo* Engine::loadShaderModules(std::string _shaderName) {

        auto vertShaderCode = Loader::readShaderFile(_shaderName + ".vert.spv");
        auto fragShaderCode = Loader::readShaderFile(_shaderName + ".frag.spv");

        _vertShaderModule = createShaderModule(vertShaderCode);
        _fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = _vertShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
        };

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = _fragShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
        };

        VkPipelineShaderStageCreateInfo* shaderStages = new VkPipelineShaderStageCreateInfo[2] {
            vertShaderStageInfo,
            fragShaderStageInfo
        };

        return shaderStages;
    }

    VkShaderModule Engine::createShaderModule(const std::vector<char>& shaderBinary) {

        VkShaderModule shaderModule;

        VkShaderModuleCreateInfo moduleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = shaderBinary.size(),
            .pCode = reinterpret_cast<const uint32_t*>(shaderBinary.data())
        };

        if (vkCreateShaderModule(_vkDevice, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
            Debug::errorWindow(L"failed to create shader module!");

        return shaderModule;
    }

//End Pass

//Graphics Pipeline Pass

    // Note: While most of the pipeline state needs to be baked into the pipeline state, a limited amount of the state can actually be changed without recreating the pipeline at draw time. Examples are the size of the viewport, line width and blend constants. If you want to use dynamic state and keep these properties out, then you'll have to fill in a VkPipelineDynamicStateCreateInfo structure like this

    VkPipelineDynamicStateCreateInfo Engine::pipelineDynamicState() {

        VkDynamicState* dynamicStates = new VkDynamicState[2] {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamicStates
        };

        return dynamicState;
    }

    VkPipelineVertexInputStateCreateInfo Engine::inputVertexState() {

        VkPipelineVertexInputStateCreateInfo vertexInputInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr, // Optional
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr, // Optional
        };

        return vertexInputInfo;
    }

    VkPipelineInputAssemblyStateCreateInfo Engine::inputAssemblyState() {
        
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        return inputAssemblyInfo;
    }

    VkPipelineViewportStateCreateInfo Engine::viewportState() {

        VkViewport* viewport = new VkViewport {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)_swapChainExtent.width,
            .height = (float)_swapChainExtent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D* scissor = new VkRect2D {
            .offset = { 0, 0 },
            .extent = _swapChainExtent
        };

        VkPipelineViewportStateCreateInfo viewportState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = viewport,
            .scissorCount = 1,
            .pScissors = scissor
        };

        return viewportState;
    }

    VkPipelineRasterizationStateCreateInfo Engine::rasterizationState() {

        VkPipelineRasterizationStateCreateInfo rasterizerState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };

        return rasterizerState;
    }

    VkPipelineMultisampleStateCreateInfo  Engine::multisamplingState() {

        VkPipelineMultisampleStateCreateInfo multisampleState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        return multisampleState;
    }

    VkPipelineColorBlendStateCreateInfo Engine::colorBlendState() {
        
        VkPipelineColorBlendAttachmentState* colorBlendAttachment = new VkPipelineColorBlendAttachmentState {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlending {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = colorBlendAttachment
        };

        colorBlending.blendConstants[0] = 0.0f; colorBlending.blendConstants[1] = 0.0f; 
        colorBlending.blendConstants[2] = 0.0f; colorBlending.blendConstants[3] = 0.0f;

        return colorBlending;
    }

    void Engine::createRenderPass() {
        
        VkAttachmentDescription colorAttachment {
            .flags = 0,
            .format = _swapChainImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpassDescription{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = 0,
            .pDependencies = nullptr
        };

        if (vkCreateRenderPass(_vkDevice, &renderPassInfo, nullptr, &_vkRenderPass) != VK_SUCCESS)
            Debug::errorWindow(L"failed to create render pass!");
    }

    void Engine::createPipelineLayout() {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 0, // Optional
            .pSetLayouts = nullptr, // Optional
            .pushConstantRangeCount = 0, // Optional
            .pPushConstantRanges = nullptr // Optional
        };

        if (vkCreatePipelineLayout(_vkDevice, &pipelineLayoutInfo, nullptr, &_vkPipelineLayout) != VK_SUCCESS)
            Debug::errorWindow(L"failed to create pipeline layout!");
    }

    void Engine::createPipeline() {

        auto shaderStages = loadShaderModules("simpletriangleShader");
        auto vertex = inputVertexState();
        auto assembly = inputAssemblyState();
        //Tessellation would have go here;
        auto viewport = viewportState();
        auto rasterization = rasterizationState();
        auto multisampling = multisamplingState();
        //DepthStencil would have go here;
        auto colorBlend = colorBlendState();
        auto dynamic = pipelineDynamicState();

        createRenderPass();
        createPipelineLayout();

        VkGraphicsPipelineCreateInfo pipelineInfo {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertex,
            .pInputAssemblyState = &assembly,
            .pTessellationState = nullptr,
            .pViewportState = &viewport,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlend,
            .pDynamicState = &dynamic,
            .layout = _vkPipelineLayout,
            .renderPass = _vkRenderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        if (vkCreateGraphicsPipelines(_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_vkGraphicsPipeline) != VK_SUCCESS)
            Debug::errorWindow(L"Error creating Graphics Pipeline!");

        //Once created the pipeline we need to destroy the variables created on the heap.
        delete[] shaderStages;
        delete viewport.pViewports;
        delete viewport.pScissors;
        delete colorBlend.pAttachments;
        delete[] dynamic.pDynamicStates;
    }
    
//End Pass

//Buffer Pass

    void Engine::createFramebuffers() {

        _swapChainFramebuffers.resize(_swapChainImageViews.size());

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = _vkRenderPass,
            .attachmentCount = 1,
            .pAttachments = nullptr,
            .width = _swapChainExtent.width,
            .height = _swapChainExtent.height,
            .layers = 1
        };

        for (size_t i = 0; i < _swapChainImageViews.size(); i++) {

            VkImageView attachment[] = { _swapChainImageViews[i] };

            framebufferInfo.pAttachments = attachment;

            if (vkCreateFramebuffer(_vkDevice, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS)
                Debug::errorWindow(L"failed to create framebuffer!");
        }
    }

//End Pass
}