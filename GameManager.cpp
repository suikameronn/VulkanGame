#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//�Q�[���S�̂̏���������
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//�w�肵��fps����A��t���[���ɂ������鎞�Ԃ��v�Z����

    glfwGetWindowSize(window, &window_width, &window_height);

    uiProjection = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f, -1.0f, 1.0f);

    vulkan->initVulkan();//Vulkan�̏�����

    FontManager* fontmanager = FontManager::GetInstance();

    //setLoadUI();//���[�hUI��ݒ�

    bool load = false;
    //drawLoading(load);

    load = createScene();//lua�X�N���v�g��ǂݎ��A�V�[���̍쐬

    //�V�[���̍쐬���I���܂ŁA�ҋ@����
    //ThreadPool::GetInstance()->stopMainThreadSingle();

    mainGameLoop();//�Q�[���̃��C�����[�v���J�n
}

//���[�hUI�̐ݒ�
void GameManager::setLoadUI()
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    std::shared_ptr<UI> ui = std::shared_ptr<UI>(new UI(FileManager::GetInstance()->loadImage("textures/loadUI.png")));

    ui->setScale(100.0f);
    ui->setPosition(glm::vec2(window_width - ui->getTexWidth(), window_height - ui->getTexHeight()));

    ui->updateTransformMatrix();
    Storage::GetInstance()->setLoadUI(ui);

    //gpu��Ƀ��[�hUI�̃e�N�X�`���Ȃǂ�W�J����
    vulkan->setUI(ui);
}

//���[�hUI��`�悷��
void GameManager::drawLoading(bool& loadFinish)
{
    //�񓯊��ŕ`�悳����
    std::function<void()> function = [this,&loadFinish]()
        {
            ThreadPool* pool = ThreadPool::GetInstance();

            VulkanBase* vulkan = VulkanBase::GetInstance();
            Storage* storage = Storage::GetInstance();

            auto start = std::chrono::system_clock::now();//�t���[���̊J�n���Ԃ��v��

            Rotate2D rot2D;
            rot2D.z = 0.0f;

            std::shared_ptr<UI> loadUI = storage->getLoadUI();

            while (!loadFinish)
            {
                //UI����]������
                loadUI->setRotate(rot2D);
                loadUI->updateTransformMatrix();

                //UI��`�悷��
                //vulkan->drawLoading();

                //UI�̊p�x�𒲐�����
                rot2D.z++;

                loadUI->frameEnd();

                //fps�������|����
                auto end = std::chrono::system_clock::now();//�t���[�����Ԃ̏I�����Ԃ��v��
                float elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());//�t���[���Ԃ̎��Ԃ��v�Z
                if (elapsed < this->frameDuration)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(this->frameDuration - elapsed)));//fps�𐧌�����
                }
                start = std::chrono::system_clock::now();
            }

            //���[�h��ʂ̕`�����߂�
            vulkan->stopLoading();
        };

    //���[�hUI�̊J�n
    ThreadPool::GetInstance()->runSingle(function);
}

bool GameManager::createScene()//�V�[�����쐬����
{
    scene = Scene::GetInstance();
    scene->init("LuaScripts/scene.lua");//lua�X�N���v�g��ǂݎ��V�[�����쐬����

    return true;
}

void GameManager::mainGameLoop()//���C���Q�[�����[�v
{
    start = std::chrono::system_clock::now();//�t���[�����Ԃ��v��

    while (true)
    {

        exit = scene->UpdateScene();//�V�[���S�̂��X�V����

        if (exit == GAME_FINISH || exit == GAME_RESTART || glfwWindowShouldClose(window))//�V�[�����I�����邩�ǂ���
        {
            break;
        }

        end = std::chrono::system_clock::now();//�t���[���I�����Ԃ��v��
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();//�t���[���Ԏ��Ԃ��v�Z����
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//fps�𐧌�����
        }
        else
        {
            std::cout << "fps down" << std::endl;
        }
        start = std::chrono::system_clock::now();

        Controller::GetInstance()->initInput();//�L�[���͂�������
        glfwPollEvents();//�L�[���͂Ȃǂ��󂯎��
    }

    exitScene();
}

//�V�[����������鏈��
void GameManager::exitScene()
{
    //�V�[����j������
    if (scene)
    {
        scene->Destroy();
    }

    //�x�������Ĕj������\��̃o�b�t�@�����ׂĔj������
    VulkanBase::GetInstance()->allCleanupDefferedBuffer();

    if (exit == GAME_FINISH || glfwWindowShouldClose(window))
    {
        //�Q�[�����I��������
        FinishGame();
    }
}

//�Q�[���S�̂̏I������
void GameManager::FinishGame()
{
    FinishInstance();
}