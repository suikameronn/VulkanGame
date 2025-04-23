#define _CRT_SECURE_NO_WARNINGS

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include<iostream>

#include"GameManager.h"

GLFWwindow* window;

#ifdef _DEBUG
int main() 
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//メモリリーク検出用
    //_CrtSetBreakAlloc(28744);

    glfwInit();//ライブラリの準備
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);//ウィンドウの作成

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fpsコントロールやゲームループ

    gameManager->initGame();//ゲームループの開始

    return EXIT_SUCCESS;
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fpsコントロールやゲームループ

    glfwInit();//ライブラリの準備
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);//ウィンドウの作成

    gameManager->initGame();//ゲームループの開始

    return EXIT_SUCCESS;
}
#endif