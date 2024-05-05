#include"GameManager.h"

void GameManager::GameLoop()
{
    vulkanBase->prepareVulkan();
    vulkanBase->defaultSetVulkan();

    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene(fileManager.get()));

    vulkanBase->setVertexIndex(scene->getSceneModel());
    vulkanBase->last();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        vulkanBase->render();
    }
}