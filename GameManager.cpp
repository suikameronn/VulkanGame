#include"GameManager.h"

#include"Scene.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()
{
    VulkanBase::GetInstance()->initVulkan();

    while (true)
    {
        //‰½‚ç‚©‚Ì“ü—Í‚ð‘Ò‚Â
        break;
    }

    createScene();
}

void GameManager::createScene()
{
    scene = std::unique_ptr<Scene>(new Scene());

    mainGameLoop();
}

void GameManager::mainGameLoop()
{

    while (!exit)
    {
        exit = scene->Update();
        VulkanBase::GetInstance()->render();
        glfwPollEvents();
    }

    exitScene();
}

void GameManager::exitScene()
{
    scene.reset();
}

void GameManager::FinishGame()
{
    glfwWindowShouldClose(window);
    FinishInstance();
}