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

		void createInstance();
		void createSwapChain(GLFWwindow* _window);

		void destroyEngine();

	private:
		
//Instance Pass

		VkInstance _vkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

//End Pass

//SwapChain Pass

		void createSurface(GLFWwindow* _window);

		void pickPhysicalDevice();

		int rateDeviceSuitability(VkPhysicalDevice device);

		void createLogicalDevice();

		void findQueueFamilies();

		bool checkDeviceExtensionSupport();

		void startSwapChain(GLFWwindow* _window);

		void createImageViews();

		SwapChainSupportDetails querySwapChainSupport();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* _window);

		VkSwapchainKHR _vkSwapChain;

		VkDevice _vkDevice = VK_NULL_HANDLE;
		VkSurfaceKHR _vkSurface = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		VkQueue _presentQueue = VK_NULL_HANDLE;

		std::vector<VkImage> _swapChainImages;
		std::vector<VkImageView> _swapChainImageViews;

		QueueFamilyIndices indices{};

//End Pass

//Graphics Pipeline Pass

		VkShaderModule createShaderModule(const std::vector<char>& shaderBinary);
		
		std::vector<VkPipelineShaderStageCreateInfo> loadShaderModules(std::string _shaderName);

		void createDynamicState();


		VkShaderModule _vertShaderModule;
		VkShaderModule _fragShaderModule;

//End Pass
	};
}