#include"GameManager.h"

#include"Scene.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::GameLoop()
{

    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());

    VulkanBase::GetInstance()->initVulkan();
    /*
    while (/*!glfwWindowShouldClose(window))
    {
    }
    */

    VulkanBase::GetInstance()->render();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}