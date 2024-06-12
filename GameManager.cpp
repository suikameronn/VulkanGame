#include"GameManager.h"

#include"Scene.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()
{
    frameDuration = (1.0f / fps) * 1000.0f;

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

    while (true)
    {
        start = std::chrono::system_clock::now();

        exit = scene->UpdateScene();
        if (exit)
        {
            std::cout << "pose" << std::endl;
        }

        VulkanBase::GetInstance()->render();

        if (exit || glfwWindowShouldClose(window))
        {
            break;
        }

        end = std::chrono::system_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));
        }
        else
        {
            std::cout << "fps down" << std::endl;
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