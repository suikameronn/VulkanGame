#include"GameManager.h"

void GameManager::GameLoop()
{
    vulkanBase->prepareVulkan();

    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene(fileManager.get()));

    vulkanBase->setModel(scene->getSceneModel());
    vulkanBase->last();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        vulkanBase->render();
    }
}