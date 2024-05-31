#include"GameManager.h"

#include"Scene.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()
{
    frameDuration = (100 / fps) * 1000;

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
    start2 = std::chrono::system_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        start = std::chrono::system_clock::now();

        exit = scene->UpdateScene();
        VulkanBase::GetInstance()->render();

        if (exit)
        {
            break;
        }

        end = std::chrono::system_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 1000;
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(frameDuration - elapsed));
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