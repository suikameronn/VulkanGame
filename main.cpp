#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include<iostream>

#include"GameManager.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* window;

int main() {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//���������[�N���o�p
    _CrtSetBreakAlloc(28744);

#endif

    glfwInit();//���C�u�����̏���
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//�E�B���h�E�̍쐬


    GameManager* gameManager = GameManager::GetInstance();//GameManager::fps�R���g���[����Q�[�����[�v
    gameManager->initGame();//�Q�[�����[�v�̊J�n

    return EXIT_SUCCESS;
}