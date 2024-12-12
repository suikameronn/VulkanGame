#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()
{
    frameDuration = (1.0f / fps) * 1000.0f;

    VulkanBase::GetInstance()->initVulkan(limitBoneDataSize);

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
    scene = std::shared_ptr<Scene>(new Scene());
    scene->init("C:/Users/sukai/Documents/VulkanGame/LusScripts/test.lua");

    mainGameLoop();
}

void GameManager::mainGameLoop()
{

    start = std::chrono::system_clock::now();

    while (true)
    {

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

            start = std::chrono::system_clock::now();

            std::cout << "aaa" << std::endl;
        }
        else
        {
            std::cout << "fps down" << std::endl;

            start = std::chrono::system_clock::now();
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