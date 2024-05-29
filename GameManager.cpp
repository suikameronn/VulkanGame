#include"GameManager.h"

#include"Scene.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()
{
    VulkanBase::GetInstance()->initVulkan();

    Controller::GetInstance();

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

    while (!glfwWindowShouldClose(window))
    {
        exit = scene->UpdateScene();
        VulkanBase::GetInstance()->render();

        if (exit)
        {
            break;
        }

        Controller::GetInstance()->initInput();
        glfwPollEvents();
    }

    exitScene();
}

void GameManager::exitScene()
{
    scene.reset();

    FinishGame();
}

void GameManager::FinishGame()
{
    FinishInstance();
}