#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//�������ݒ�
{
    frameDuration = (1.0f / fps) * 1000.0f;//�ݒ肳�ꂽ�t���[�����[�g����A1�t���[���ł̍Œ�̏������Ԃ�ݒ�

    VulkanBase::GetInstance()->initVulkan();//Vulkan�̃f�[�^�̈ꕔ�����炩���ߗp�ӂ��Ă���

    createScene();//�X�e�[�W�̓ǂݍ���
}

void GameManager::createScene()//lua����X�e�[�W�̗l�q��ǂݍ���
{
    scene = Scene::GetInstance();//lua����ǂݎ�����f�[�^����X�e�[�W�̗l�q�����N���X
    scene->init("LuaScripts/test.lua");//�X�e�[�W�̓ǂݍ���

    mainGameLoop();//�Q�[�����[�v
}

void GameManager::mainGameLoop()//���C���Q�[�����[�v
{

    start = std::chrono::system_clock::now();//�t���[���J�n����

    while (true)
    {

        exit = scene->UpdateScene();//�Q�[�����I������ꍇ�Atrue���Ԃ�

        VulkanBase::GetInstance()->render();//�I�u�W�F�N�g�̃����_�����O

        if (exit || glfwWindowShouldClose(window))//���C���Q�[�����[�v�̏I������
        {
            break;
        }

        end = std::chrono::system_clock::now();//�t���[���I������
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();//�t���[���Ԏ��Ԍv��
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//�������~����
        }
        else
        {
            std::cout << "fps down" << std::endl;
        }
        start = std::chrono::system_clock::now();

        Controller::GetInstance()->initInput();//�L�[�{�[�h������͂�������
        glfwPollEvents();//�L�[�{�[�h������͂��擾
    }

    exitScene();
}

//�X�e�[�W���o��
void GameManager::exitScene()
{
    //�X�e�[�W��j��
    scene->Destroy();

    //�Q�[���S�̂��I��
    FinishGame();
}

//�Q�[���S�̂̏I������
void GameManager::FinishGame()
{
    FinishInstance();
}