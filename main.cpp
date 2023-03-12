#include "VulkanEngine.hpp"

int main()
{
    EggyEngine::Engine* _vkEngine = new EggyEngine::Engine;
    
    try {
        _vkEngine->run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    delete _vkEngine;

    return EXIT_SUCCESS;
}