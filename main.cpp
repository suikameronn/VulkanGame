#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include<iostream>

#include"GameManager.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* window;

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//���������[�N���o�p

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);


    GameManager* gameManager = GameManager::GetInstance();
    gameManager->initGame();

    std::cout << "main function" << _CrtDumpMemoryLeaks() << std::endl;

    return EXIT_SUCCESS;
}