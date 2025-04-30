#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//�������ݒ�
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//�ݒ肳�ꂽ�t���[�����[�g����A1�t���[���ł̍Œ�̏������Ԃ�ݒ�

    vulkan->initVulkan();//Vulkan�̃f�[�^�̈ꕔ�����炩���ߗp�ӂ��Ă���

    setLoadUI();//���[�hUI�̐ݒ�

    bool load = false;
    drawLoading(load);

    load = createScene();//�X�e�[�W�̓ǂݍ���

    //���[�h��ʕ`��̏I����҂�
    ThreadPool::GetInstance()->stopMainThreadSingle();

    mainGameLoop();//�Q�[�����[�v
}

//���[�hUI�̐ݒ�
void GameManager::setLoadUI()
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    std::shared_ptr<UI> ui = std::shared_ptr<UI>(new UI(FileManager::GetInstance()->loadImage("textures/loadUI.png")));

    ui->setScale(50.0f);
    ui->setPosition(glm::vec2(window_width - ui->getTexWidth(), window_height - ui->getTexHeight()));

    ui->updateTransformMatrix();
    Storage::GetInstance()->setLoadUI(ui);

    //gpu��ɒ��_�o�b�t�@�Ȃǂ��쐬
    vulkan->setUI(ui);
}

//���[�hUI�̕\��
void GameManager::drawLoading(bool& loadFinish)
{
    //�ʃX���b�h�ɓn���A�����p�̕ϐ�
    inputTypes settings(2);
    settings[0] = &loadFinish;
    settings[1] = frameDuration;

    //���[�h��ʂ̕`��(�ʃX���b�h�Ŏ��s)
    std::function<void(inputTypes&)> function = [](std::vector<std::any>& loadSettings)
        {
            //�z�񂩂�ݒ���擾����
            bool* loadFinish = nullptr;
            float frameDuration = 0.0f;
            ThreadPool* pool = ThreadPool::GetInstance();
            pool->safeAnyCast(loadFinish, 0, loadSettings);
            pool->safeAnyCast(frameDuration, 1, loadSettings);

            VulkanBase* vulkan = VulkanBase::GetInstance();
            Storage* storage = Storage::GetInstance();

            auto start = std::chrono::system_clock::now();//�t���[���J�n����

            Rotate2D rot2D;
            rot2D.z = 0.0f;

            while (!*loadFinish)
            {
                //UI�̉�]�s����v�Z
                storage->getLoadUI()->setRotate(rot2D);
                storage->getLoadUI()->updateTransformMatrix();

                //UI�̕`��
                vulkan->drawLoading();

                //UI�̉�]
                rot2D.z++;

                //fps����
                auto end = std::chrono::system_clock::now();//�t���[���I������
                float elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());//�t���[���Ԏ��Ԍv��
                if (elapsed < frameDuration)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//�������~����
                }
                start = std::chrono::system_clock::now();
            }

            //���[�h��ʃ����_�����O�̌�n��������
            vulkan->stopLoading();
        };

    //�ʃX���b�h�Ŏ��s�J�n
    std::pair<inputTypes, std::function<void(inputTypes&)>> pair = { settings,function };
    ThreadPool::GetInstance()->runSingle(pair);
}

bool GameManager::createScene()//lua����X�e�[�W�̗l�q��ǂݍ���
{
    scene = Scene::GetInstance();//lua����ǂݎ�����f�[�^����X�e�[�W�̗l�q�����N���X
    scene->init("LuaScripts/scene.lua");//�X�e�[�W�̓ǂݍ���

    return true;
}

void GameManager::mainGameLoop()//���C���Q�[�����[�v
{
    start = std::chrono::system_clock::now();//�t���[���J�n����

    while (true)
    {

        exit = scene->UpdateScene();//�Q�[�����I������ꍇ�Atrue���Ԃ�

        VulkanBase::GetInstance()->render();//�I�u�W�F�N�g�̃����_�����O

        if (exit == GAME_FINISH || exit == GAME_RESTART || glfwWindowShouldClose(window))//���C���Q�[�����[�v�̏I������
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
    if (scene)
    {
        scene->Destroy();
    }

    if (exit == GAME_FINISH || glfwWindowShouldClose(window))
    {
        //�Q�[���S�̂��I��
        FinishGame();
    }
    else if(exit == GAME_RESTART)
    {
        RestartGame();
    }
}

//�Q�[���S�̂̏I������
void GameManager::FinishGame()
{
    FinishInstance();
}

//�Q�[���̃��X�^�[�g���� �ǂݎ�������\�[�X�͉�����Ȃ�
void GameManager::RestartGame()
{
    VulkanBase::GetInstance()->FinishVulkanBase();

    initGame();
}