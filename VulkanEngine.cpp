#include "VulkanEngine.hpp"

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace EggyEngine {

	void Engine::destroyEngine() {

        vkDestroyShaderModule(_vkDevice, _fragShaderModule, nullptr);

        vkDestroyShaderModule(_vkDevice, _vertShaderModule, nullptr);

        for (auto imageView : _swapChainImageViews)
            vkDestroyImageView(_vkDevice, imageView, nullptr);

        vkDestroySwapchainKHR(_vkDevice, _vkSwapChain, nullptr);

        vkDestroySurfaceKHR(_vkInstance, _vkSurface, nullptr);

        vkDestroyDevice(_vkDevice, nullptr);

		if (enableValidationLayers)
            Debug::destroyDebugUtilsMessengerEXT(_vkInstance, _debugMessenger, nullptr);

		vkDestroyInstance(_vkInstance, nullptr);
	}

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

        if (Debug::CreateDebugUtilsMessengerEXT(_vkInstance, &debugCreateInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
            Debug::errorWindow(L"validation enabled but failed to create debug messenger!");
	    
    }

//End Pass

//SwapChain Pass

    void Engine::createSwapChain(GLFWwindow* _window) {
        
        createSurface(_window);

        pickPhysicalDevice();

        createLogicalDevice();

        startSwapChain(_window);

        createImageViews();

        auto shaderStages = loadShaderModules("simpletriangleShader");
    }

    void Engine::createSurface(GLFWwindow* _window) {

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

    void Engine::createLogicalDevice() {

        findQueueFamilies();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily,
            indices.presentFamily
        };

        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies) {

            VkDeviceQueueCreateInfo queueCreateInfo{

                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority,
            };

            queueCreateInfos.push_back(queueCreateInfo);
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

    void Engine::startSwapChain(GLFWwindow* _window) {

        SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, _window);

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

        for (size_t i = 0; i < _swapChainImages.size(); i++) {

            VkImageViewCreateInfo viewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = _swapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = _swapChainImageFormat,
            };

            viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewCreateInfo.subresourceRange.baseMipLevel = 0;
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.subresourceRange.baseArrayLayer = 0;
            viewCreateInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_vkDevice, &viewCreateInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
                Debug::errorWindow(L"failed to create image views!");
        }
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

    VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* _window) {

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

//End Pass

//Graphics Pipeline Pass
    
    std::vector<VkPipelineShaderStageCreateInfo> Engine::loadShaderModules(std::string _shaderName) {

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
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = _fragShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
        };

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
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

    void Engine::createDynamicState() {

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr, // Optional
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr, // Optional
        };

    }

//End Pass
}