#include "vkWindow.hpp"

namespace EggyEngine {

	Window::Window()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_vkWindow = glfwCreateWindow(WIDTH, HEIGHT, "9/11 was a inside job",
			nullptr, //No Fullscreen
			nullptr  //No Shared Resources 
		);

		if (_vkWindow == nullptr)
			Debug::errorWindow(L"Unable to create glfw window");

	}

	Window::~Window()
	{
		_vkEngine.destroyEngine();

		glfwDestroyWindow(_vkWindow);
		glfwTerminate();
	}

	void Window::startEngine() {
		
		_vkEngine.createInstance();

		_vkEngine.createSwapChain(_vkWindow);
	}

	void Window::runWindow()
	{

		while (true) {
			if (glfwWindowShouldClose(_vkWindow))
				return;

			glfwPollEvents();
		}
	}
}