#include"GameManager.h"

void GameManager::GameLoop()
{
    vulkanBase->prepareVulkan();
    vulkanBase->defaultSetVulkan();

    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        vulkanBase->setVertexIndex(scene->getSceneModel());
        vulkanBase->last();
        vulkanBase->render();
    }
}