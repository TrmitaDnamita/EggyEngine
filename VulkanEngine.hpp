#include "HelperNamespaces.hpp"

struct QueueFamilyIndices {
	uint32_t graphicsFamily = 0;
	uint32_t presentFamily = 0;

	VkBool32 graphicsFamilySet = false;
	VkBool32 presentFamilySet = false;

	bool isComplete() { return (graphicsFamilySet && presentFamilySet); }

	bool indicesMatch() { return graphicsFamily == presentFamily; }
};

struct SwapChainSupportDetails {

	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

namespace EggyEngine{
	
	class Engine {
	public:

		Engine();
		~Engine();

		void run();

	private:
		
		void destroyWindow();
		void destroyInstance();
		void destroySwapChain();
		void destroyPipeline();

//Window Pass

		void startEngine();

		void createInstance();

		void createSwapChain();

		void createPipeline();

		GLFWwindow* _window = nullptr;

//End Pass

//Instance Pass

		VkInstance _vkInstance = VK_NULL_HANDLE;

//End Pass

//SwapChain Pass

		void createSurface();

		void pickPhysicalDevice();

		int rateDeviceSuitability(VkPhysicalDevice device);

		void createLogicalDevice();

		void findQueueFamilies();

		bool checkDeviceExtensionSupport();

		void startSwapChain();

		void createImageViews();

		SwapChainSupportDetails querySwapChainSupport();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkDevice _vkDevice = VK_NULL_HANDLE;
		VkSurfaceKHR _vkSurface = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

		VkSwapchainKHR _vkSwapChain = VK_NULL_HANDLE;

		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;

		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		VkQueue _presentQueue = VK_NULL_HANDLE;

		std::vector<VkImage> _swapChainImages;
		std::vector<VkImageView> _swapChainImageViews;

		QueueFamilyIndices indices{};

//End Pass

//Graphics Pipeline Pass

		VkShaderModule createShaderModule(const std::vector<char>& shaderBinary);
		
		VkPipelineShaderStageCreateInfo* loadShaderModules(std::string _shaderName);

		VkPipelineDynamicStateCreateInfo pipelineDynamicState();
		VkPipelineVertexInputStateCreateInfo inputVertexState();
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState();

		VkPipelineViewportStateCreateInfo viewportState();
		VkPipelineRasterizationStateCreateInfo rasterizationState();
		VkPipelineMultisampleStateCreateInfo  multisamplingState();
		VkPipelineColorBlendStateCreateInfo colorBlendState();

		void createRenderPass();
		void createPipelineLayout();
		
		VkRenderPass _vkRenderPass = VK_NULL_HANDLE;
		VkPipelineLayout _vkPipelineLayout = VK_NULL_HANDLE;
		VkPipeline _vkGraphicsPipeline = VK_NULL_HANDLE;

		VkShaderModule _vertShaderModule = VK_NULL_HANDLE;
		VkShaderModule _fragShaderModule = VK_NULL_HANDLE;

//End Pass

//Draw Pass
		
		void drawFrame();
		void createSyncObjects();

		void destroyDraw();

		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffer();

		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		std::vector<VkFramebuffer> _swapChainFramebuffers;

		VkCommandPool _vkCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer _vkCommandBuffer = VK_NULL_HANDLE;
		
		VkSemaphore _vkImageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore _vkRenderFinishedSemaphore = VK_NULL_HANDLE;
		VkFence _vkInFlightFence = VK_NULL_HANDLE;

//End Pass

	};
}