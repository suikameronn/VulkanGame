#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include<iostream>

#include"GameManager.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* window;

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//メモリリーク検出用
    //_CrtSetBreakAlloc(28744);


    glfwInit();//ライブラリの準備
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//ウィンドウの作成


    GameManager* gameManager = GameManager::GetInstance();//GameManager::fpsコントロールやゲームループ
    gameManager->initGame();//ゲームループの開始

    return EXIT_SUCCESS;
}