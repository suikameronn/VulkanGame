#include"GameManager.h"

void GameManager::GameLoop()
{
    vulkanBase->prepareVulkan();

    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene(fileManager.get()));

    //�e�N�X�`���̔z����ۂ��Ƃق���
    vulkanBase->prepareTextureVulkan(fileManager->getRegisteredImageCount());
    vulkanBase->setModel(scene->getSceneModel());
    vulkanBase->last();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        vulkanBase->render();
    }
}