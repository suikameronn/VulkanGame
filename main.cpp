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
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//���������[�N���o�p
    //_CrtSetBreakAlloc(28744);

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fps�R���g���[����Q�[�����[�v

    glfwInit();//���C�u�����̏���
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(gameManager->window_width, gameManager->window_height, "Vulkan", nullptr, nullptr);//�E�B���h�E�̍쐬

    gameManager->initGame();//�Q�[�����[�v�̊J�n

    return EXIT_SUCCESS;
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fps�R���g���[����Q�[�����[�v

    glfwInit();//���C�u�����̏���
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(gameManager->window_width, gameManager->window_height, "Vulkan", nullptr, nullptr);//�E�B���h�E�̍쐬

    gameManager->initGame();//�Q�[�����[�v�̊J�n

    return EXIT_SUCCESS;
}
#endif