#include"GameManager.h"

#include"Scene.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::GameLoop()
{
    VulkanBase::GetInstance()->prepareVulkan();

    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());

    //�e�N�X�`���̔z����ۂ��Ƃق���
    VulkanBase::GetInstance()->last();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        VulkanBase::GetInstance()->render();
    }
}