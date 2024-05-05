#include<iostream>

#include"GameManager.h"
#include"VulkanBase.h"
#include"Controller.h"
#include"FileManager.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//メモリリーク検出用

    GLFWwindow* window;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    std::shared_ptr<Controller> controller = std::shared_ptr<Controller>(new Controller(window));
    std::shared_ptr<VulkanBase> vulkan = std::shared_ptr<VulkanBase>(new VulkanBase(window));
    std::shared_ptr<FileManager> manager = std::shared_ptr<FileManager>(new FileManager());

    std::unique_ptr<GameManager> gameManager
        = std::unique_ptr <GameManager>(new GameManager(manager, vulkan, controller, window));

    gameManager->GameLoop();

    return EXIT_SUCCESS;
}