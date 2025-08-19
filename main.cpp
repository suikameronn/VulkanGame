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
    //_CrtSetBreakAlloc(113);

    glfwInit();//���C�u�����̏���
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // �v���C�}�����j�^���擾
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        std::cerr << "�v���C�}�����j�^���擾�ł��܂���ł����B" << std::endl;
        glfwTerminate();
        return -1;
    }

    // �v���C�}�����j�^�̃r�f�I���[�h���擾
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    if (!mode) {
        std::cerr << "�v���C�}�����j�^�̃r�f�I���[�h���擾�ł��܂���ł����B" << std::endl;
        glfwTerminate();
        return -1;
    }

    // �E�B���h�E�q���g��ݒ� (�{�[�_���X)
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    //window = glfwCreateWindow(mode->width, mode->height, "Vulkan", nullptr, nullptr);//�E�B���h�E�̍쐬
    window = glfwCreateWindow(500,300, "Vulkan", nullptr, nullptr);//�E�B���h�E�̍쐬

    // �E�B���h�E�ʒu��������ɐݒ�
    //glfwSetWindowPos(window, 0, 0);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fps�R���g���[����Q�[�����[�v
    gameManager->setWindow(window);

    gameManager->initGame();//�Q�[�����[�v�̊J�n

    delete gameManager;

    std::cout << "Finish" << std::endl;

    return EXIT_SUCCESS;
}
#else

#include<Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    glfwInit();//���C�u�����̏���
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // �v���C�}�����j�^���擾
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        std::cerr << "�v���C�}�����j�^���擾�ł��܂���ł����B" << std::endl;
        glfwTerminate();
        return -1;
    }

    // �v���C�}�����j�^�̃r�f�I���[�h���擾
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    if (!mode) {
        std::cerr << "�v���C�}�����j�^�̃r�f�I���[�h���擾�ł��܂���ł����B" << std::endl;
        glfwTerminate();
        return -1;
    }

    // �E�B���h�E�q���g��ݒ� (�{�[�_���X)
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    window = glfwCreateWindow(mode->width, mode->height, "Vulkan", nullptr, nullptr);//�E�B���h�E�̍쐬

    // �E�B���h�E�ʒu��������ɐݒ�
    glfwSetWindowPos(window, 0, 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GameManager* gameManager = GameManager::GetInstance();//GameManager::fps�R���g���[����Q�[�����[�v
    gameManager->setWindow(window);

    gameManager->initGame();//�Q�[�����[�v�̊J�n

    return EXIT_SUCCESS;
}
#endif