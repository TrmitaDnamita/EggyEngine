#pragma once

#include "HelperNamespaces.hpp"
#include "VulkanEngine.hpp"

namespace EggyEngine {

	class Window {
	public:

		Window();
		~Window();

		void startEngine();

		void runWindow();

	private:

		GLFWwindow* _vkWindow;

		Engine _vkEngine;
	};
}