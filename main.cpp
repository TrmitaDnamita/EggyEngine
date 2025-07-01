#include "VulkanEngine.hpp"

int main()
{
    EggyEngine::Engine _vkEngine;
    
    try {
        _vkEngine.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    

    return EXIT_SUCCESS;
}