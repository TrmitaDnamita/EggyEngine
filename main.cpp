#include "vkWindow.hpp"

int main()
{
    try {
        EggyEngine::Window _window;

        _window.startEngine();

        _window.runWindow();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}