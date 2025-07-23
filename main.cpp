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
    //_CrtSetBreakAlloc(1597);

    glfwInit();//ライブラリの準備
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // プライマリモニタを取得
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        std::cerr << "プライマリモニタを取得できませんでした。" << std::endl;
        glfwTerminate();
        return -1;
    }

    // プライマリモニタのビデオモードを取得
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    if (!mode) {
        std::cerr << "プライマリモニタのビデオモードを取得できませんでした。" << std::endl;
        glfwTerminate();
        return -1;
    }

    // ウィンドウヒントを設定 (ボーダレス)
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    window = glfwCreateWindow(mode->width, mode->height, "Vulkan", nullptr, nullptr);//ウィンドウの作成

    // ウィンドウ位置を左上隅に設定
    glfwSetWindowPos(window, 0, 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fpsコントロールやゲームループ

    gameManager->initGame();//ゲームループの開始

    std::cout << "Finish" << std::endl;

    return EXIT_SUCCESS;
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    glfwInit();//ライブラリの準備
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // プライマリモニタを取得
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        std::cerr << "プライマリモニタを取得できませんでした。" << std::endl;
        glfwTerminate();
        return -1;
    }

    // プライマリモニタのビデオモードを取得
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    if (!mode) {
        std::cerr << "プライマリモニタのビデオモードを取得できませんでした。" << std::endl;
        glfwTerminate();
        return -1;
    }

    // ウィンドウヒントを設定 (ボーダレス)
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    window = glfwCreateWindow(mode->width, mode->height, "Vulkan", nullptr, nullptr);//ウィンドウの作成

    // ウィンドウ位置を左上隅に設定
    glfwSetWindowPos(window, 0, 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fpsコントロールやゲームループ

    gameManager->initGame();//ゲームループの開始

    return EXIT_SUCCESS;
}
#endif