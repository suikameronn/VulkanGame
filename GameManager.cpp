#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//�Q�[���S�̂̏���������
{
	vulkanCore = std::make_shared<VulkanCore>();

	createBuilder();//�r���_�[�̗p��
	createFactory();//�t�@�N�g���[�̗p��

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//�w�肵��fps����A��t���[���ɂ������鎞�Ԃ��v�Z����

    bool load = false;

	ecsManager = std::make_shared<ECSManager>();

	createScene();//�V�[���̍쐬

    mainGameLoop();//�Q�[���̃��C�����[�v���J�n
}

//�r���_�[�̗p��
void GameManager::createBuilder()
{
	descriptorSetBuilder = std::make_shared<DescriptorSetBuilder>(vulkanCore);
	frameBufferBuilder = std::make_shared<FrameBufferBuilder>(vulkanCore);
	bufferBuilder = std::make_shared<GpuBufferBuilder>(vulkanCore);
	descriptorSetLayoutBuilder = std::make_shared<DescriptorSetLayoutBuilder>(vulkanCore);
	pipelineLayoutBuilder = std::make_shared<PipelineLayoutBuilder>(vulkanCore);
	pipelineBuilder = std::make_shared<PipelineBuilder>(vulkanCore);
	renderPassBuilder = std::make_shared<RenderPassBuilder>(vulkanCore);
}

//�t�@�N�g���[�̗p��
void GameManager::createFactory()
{
	descriptorSetFactory = std::make_shared<DescriptorSetFactory>(vulkanCore, descriptorSetBuilder);
	frameBufferFactory = std::make_shared<FrameBufferFactory>(vulkanCore, frameBufferBuilder);
	bufferFactory = std::make_shared<GpuBufferFactory>(vulkanCore, bufferBuilder);
	descriptorSetLayoutFactory = std::make_shared<DescriptorSetLayoutFactory>(vulkanCore, descriptorSetLayoutBuilder);
	pipelineLayoutFactory = std::make_shared<PipelineLayoutFactory>(vulkanCore, pipelineLayoutBuilder);
	pipelineFactory = std::make_shared<PipelineFactory>(vulkanCore, pipelineBuilder);
	renderPassFactory = std::make_shared<RenderPassFactory>(vulkanCore, renderPassBuilder);
}

//�V�[���̍쐬
void GameManager::createScene()
{

}

void GameManager::mainGameLoop()//���C���Q�[�����[�v
{
    start = std::chrono::system_clock::now();//�t���[�����Ԃ��v��

    while (true)
    {

        if (vulkanCore->isShouldCloseWindow())//�V�[�����I�����邩�ǂ���
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
    }

    exitScene();
}

//�V�[����������鏈��
void GameManager::exitScene()
{
    //�Q�[�����I��������
    FinishGame();
}

//�Q�[���S�̂̏I������
void GameManager::FinishGame()
{

}