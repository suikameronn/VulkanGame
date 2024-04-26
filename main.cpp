#include<iostream>

#include"VulkanBase.h"
#include"Controller.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//メモリリーク検出用

    GLFWwindow* window;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    Controller controller(window);
    VulkanBase app(window);

    glfwSetWindowUserPointer(window,&controller);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    app.prepareVulkan();
    app.defaultSetVulkan();

    try
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            app.render();
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}