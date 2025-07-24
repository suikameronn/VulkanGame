#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//�Q�[���S�̂̏���������
{
	vulkanCore = std::make_shared<VulkanCore>(window);

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//�w�肵��fps����A��t���[���ɂ������鎞�Ԃ��v�Z����

    bool load = false;

	ecsManager = std::make_shared<ECSManager>();

	createInstance();//�C���X�^���X�̍쐬

	createScene();//�V�[���̍쐬

    mainGameLoop();//�Q�[���̃��C�����[�v���J�n
}

//�C���X�^���X���쐬����
void GameManager::createInstance()
{
    createBuilder();//�r���_�[�̗p��
    createFactory();//�t�@�N�g���[�̗p��

	VkDevice& device = vulkanCore->getLogicDevice();

	pipelineBuilder = std::make_shared<PipelineBuilder>(device, shaderFactory);

	textureBuilder = std::make_shared<TextureBuilder>(vulkanCore, bufferFactory);
	textureFactory = std::make_shared<TextureFactory>(device, textureBuilder);

    materialBuilder = std::make_shared<MaterialBuilder>(bufferFactory, descriptorSetLayoutFactory
        , descriptorSetFactory, textureFactory);

	modelFactory = std::make_shared<GltfModelFactory>(materialBuilder, textureFactory
		, bufferFactory, descriptorSetLayoutFactory, descriptorSetFactory);

    swapChain = std::make_shared<SwapChain>(vulkanCore, textureFactory, renderPassFactory, frameBufferFactory);

	render = std::make_shared<Render>(device);
}

//�r���_�[�̗p��
void GameManager::createBuilder()
{
	VkPhysicalDevice physicalDevice = vulkanCore->getPhysicalDevice();
	VkDevice device = vulkanCore->getLogicDevice();

	descriptorSetBuilder = std::make_shared<DescriptorSetBuilder>(device);
	frameBufferBuilder = std::make_shared<FrameBufferBuilder>(device);
	bufferBuilder = std::make_shared<GpuBufferBuilder>(physicalDevice, device);
	descriptorSetLayoutBuilder = std::make_shared<DescriptorSetLayoutBuilder>();
	pipelineLayoutBuilder = std::make_shared<PipelineLayoutBuilder>();
	renderPassBuilder = std::make_shared<RenderPassBuilder>(device);
}

//�t�@�N�g���[�̗p��
void GameManager::createFactory()
{
	VkDevice device = vulkanCore->getLogicDevice();

	frameBufferFactory = std::shared_ptr<FrameBufferFactory>(new FrameBufferFactory(device, frameBufferBuilder));

	descriptorSetLayoutFactory = std::make_shared<DescriptorSetLayoutFactory>(device, descriptorSetLayoutBuilder);
	descriptorSetFactory = std::make_shared<DescriptorSetFactory>(device, descriptorSetBuilder, descriptorSetLayoutFactory);
	bufferFactory = std::make_shared<GpuBufferFactory>(vulkanCore, bufferBuilder);
	pipelineLayoutFactory = std::make_shared<PipelineLayoutFactory>(device, pipelineLayoutBuilder, descriptorSetLayoutFactory);
	renderPassFactory = std::make_shared<RenderPassFactory>(vulkanCore, renderPassBuilder);
	shaderFactory = std::make_shared<ShaderFactory>(device);
	pipelineFactory = std::make_shared<PipelineFactory>(device, pipelineLayoutFactory, shaderFactory, pipelineBuilder, renderPassFactory);
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

		OnUpdate();//�R���|�[�l���g�̍X�V����

		OnLateUpdate();//�X�V������̏���

		Rendering();//�I�u�W�F�N�g�̃����_�����O

		OnFrameEnd();//�t���[���I�����̏���

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

//�R���|�[�l���g�̏��񏈗�
void GameManager::OnStart()
{
	//ecsManager->RunFunction<
}

//�R���|�[�l���g�̍X�V����
void GameManager::OnUpdate()
{

}

//�X�V������̏���
void GameManager::OnLateUpdate()
{

}

//�I�u�W�F�N�g�̃����_�����O
void GameManager::Rendering()
{
	//ecsManager->RunFunction<

	//���\�[�X�̔j��
	descriptorSetLayoutFactory->resourceDestruct();
	bufferFactory->resourceDestruct();
	frameBufferFactory->resourceDestruct();
	pipelineLayoutFactory->resourceDestruct();
	pipelineFactory->resourceDestruct();
	renderPassFactory->resourceDestruct();
	shaderFactory->resourceDestruct();
	textureFactory->resourceDestruct();
}

//�t���[���I�����̏���
void GameManager::OnFrameEnd()
{

}

//�V�[�����I�������鏈��
void GameManager::exitScene()
{
    //�Q�[�����I��������
    FinishGame();
}

//�Q�[���S�̂̏I������
void GameManager::FinishGame()
{

}